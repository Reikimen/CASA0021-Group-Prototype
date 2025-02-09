import 'dart:convert'; // 添加此导入以使用 UTF-8 编码器
import 'dart:async';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:geolocator/geolocator.dart';
import 'package:intl/intl.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'ESP32 BLE Connector',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const MainScreen(),
    );
  }
}

class MainScreen extends StatefulWidget {
  const MainScreen({super.key});

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> {
  final TextEditingController ssidController = TextEditingController();
  final TextEditingController passwordController = TextEditingController();

  List<BluetoothDevice> devices = [];
  BluetoothDevice? selectedDevice;
  BluetoothConnectionState connectionState = BluetoothConnectionState.disconnected;
  Position? currentPosition;
  Stream<Position>? positionStream;
  StreamSubscription<List<ScanResult>>? scanSubscription;
  StreamSubscription<BluetoothConnectionState>? _connectionStateSubscription;
  bool isDiscoveringServices = false;

  // 目标服务/特征 UUID | Target Service/Characteristic UUIDs
  static const targetServiceUuid = "0000ff01-0000-1000-8000-00805f9b34fb";
  static const wifiCharacteristicUuid = "0000ff03-0000-1000-8000-00805f9b34fb";
  static const gpsCharacteristicUuid = "0000ff02-0000-1000-8000-00805f9b34fb";

  @override
  void initState() {
    super.initState();
    _checkPermissions();
  }

  Future<void> _checkPermissions() async {
    if (Platform.isAndroid || Platform.isIOS) {
      Map<Permission, PermissionStatus> statuses = await [
        Permission.locationWhenInUse,
        Permission.bluetooth,
        Permission.bluetoothScan,
        Permission.bluetoothConnect,
      ].request();

      statuses.forEach((key, value) {
        print("Permission $key: ${value.isGranted ? 'Granted' : 'Denied'}");
        if (!value.isGranted) {
          showDialog(
            context: context,
            builder: (ctx) => AlertDialog(
              title: const Text("Insufficient Permissions"),
              content: Text("Please grant $key permission"),
              actions: [
                TextButton(
                  onPressed: () => Navigator.pop(ctx),
                  child: const Text("OK"),
                )
              ],
            ),
          );
        }
      });
    }
  }

  Future<void> _scanDevices() async {
    if (!await FlutterBluePlus.isSupported) {
      print("Bluetooth not supported");
      return;
    }

    if (FlutterBluePlus.isScanningNow) {
      await FlutterBluePlus.stopScan();
      print("Stopped previous scan");
    }

    devices.clear();
    setState(() {});

    scanSubscription = FlutterBluePlus.scanResults.listen((results) {
      for (var result in results) {
        if (!devices.any((d) => d.remoteId == result.device.remoteId)) {
          print("Found device: ${result.device.platformName} (${result.device.remoteId})");
          setState(() => devices.add(result.device));
        }
      }
    }, onError: (e) => print("Scan error: $e"));

    await FlutterBluePlus.startScan(
      timeout: const Duration(seconds: 10),
      androidUsesFineLocation: true,
    );
    print("Starting BLE scan...");
  }

  Future<void> _connectToDevice() async {
    if (selectedDevice == null) return;

    print("Connecting to device: ${selectedDevice!.remoteId}");
    try {
      // Cancel previous subscriptions
      _connectionStateSubscription?.cancel();

      // Connect to device
      await selectedDevice!.connect(timeout: const Duration(seconds: 15));

      // Listen to connection state changes
      _connectionStateSubscription = selectedDevice!.connectionState.listen((state) {
        print("Connection state updated: $state");
        setState(() => connectionState = state);

        // Handle disconnection
        if (state == BluetoothConnectionState.disconnected) {
          positionStream?.drain();
          currentPosition = null;
        }
      });

      print("Connection successful, discovering services...");
      await _startLocationUpdates();
    } catch (e) {
      print("Connection failed: $e");
      setState(() => connectionState = BluetoothConnectionState.disconnected);
    }
  }

  Future<void> _startLocationUpdates() async {
    positionStream = Geolocator.getPositionStream(
      locationSettings: const LocationSettings(
        accuracy: LocationAccuracy.high,
        distanceFilter: 10,
      ),
    );

    positionStream!.listen((Position position) {
      if (connectionState != BluetoothConnectionState.connected) {
        print("Skipping location update: Device not connected");
        return;
      }
      print("New position: $position");
      setState(() => currentPosition = position);
      _sendLocationData();
    });
  }

// 修改 _sendData 方法
  Future<void> _sendData(String characteristicUuid, String data) async {
    if (selectedDevice == null ||
        connectionState != BluetoothConnectionState.connected) {
      print("[Error] Device not connected");
      return;
    }

    try {
      if (isDiscoveringServices) {
        print("[Warning] Service discovery in progress...");
        return;
      }
      isDiscoveringServices = true;

      // 1. 发现服务（带缓存优化）
      List<BluetoothService> services = await selectedDevice!.discoverServices();
      print("Discovered ${services.length} services");

      // 2. 查找目标服务
      final targetService = services.firstWhere(
              (s) => s.uuid.toString() == targetServiceUuid,
          orElse: () {
            print("[Error] Service $targetServiceUuid not found");
            throw 'Service not found';
          }
      );

      // 3. 查找目标特征
      final targetChar = targetService.characteristics.firstWhere(
              (c) => c.uuid.toString() == characteristicUuid,
          orElse: () {
            print("[Error] Characteristic $characteristicUuid not found");
            throw 'Characteristic not found';
          }
      );

      // 4. 验证特征可写性
      if (!targetChar.properties.write) {
        print("[Error] Characteristic is not writable");
        throw 'Write not permitted';
      }

      // 5. 使用UTF-8编码发送数据
      print("Sending data (UTF-8): $data");
      final encodedData = utf8.encode(data); // 关键修改点
      await targetChar.write(encodedData, withoutResponse: true);
      print("Data sent successfully");

    } on FlutterBluePlusException catch (e) {
      // 专用异常处理
      print("""
    BLE Operation Failed!
    Code: ${e.code}
    Description: ${e.description}
    """);
    } catch (e) {
      print("Unexpected error: $e");
    } finally {
      isDiscoveringServices = false;
    }
  }

// 修改后的发送方法
  Future<void> _sendWifiCredentials() async {
    try {
      final credentials = 'SSID:${ssidController.text},PASS:${passwordController.text}';
      print("Attempting to send WiFi credentials: $credentials");
      await _sendData(wifiCharacteristicUuid, credentials);
    } catch (e) {
      print("WiFi credential send failed: $e");
    }
  }

  Future<void> _sendLocationData() async {
    if (currentPosition == null) {
      print("[Warning] No location data available");
      return;
    }

    try {
      final locationData = 'LAT:${currentPosition!.latitude},LON:${currentPosition!.longitude}';
      print("Attempting to send GPS data: $locationData");
      await _sendData(gpsCharacteristicUuid, locationData);
    } catch (e) {
      print("GPS data send failed: $e");
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('ESP32 BLE Connector'),
        actions: [
          IconButton(
            icon: const Icon(Icons.bluetooth),
            onPressed: _checkPermissions,
          )
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: ListView(
          children: [
            _buildScanSection(),
            _buildConnectionSection(),
            _buildWifiForm(),
            _buildLocationInfo(),
          ],
        ),
      ),
    );
  }

  Widget _buildScanSection() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(12.0),
        child: Column(
          children: [
            ElevatedButton(
              onPressed: _scanDevices,
              child: const Text('Scan Devices'),
            ),
            const SizedBox(height: 10),
            Text("Found ${devices.length} devices", style: const TextStyle(color: Colors.grey)),
          ],
        ),
      ),
    );
  }

  Widget _buildConnectionSection() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(12.0),
        child: Column(
          children: [
            DropdownButton<BluetoothDevice>(
              isExpanded: true,
              hint: const Text('Select BLE Device'),
              value: selectedDevice,
              items: devices.map((device) {
                return DropdownMenuItem(
                  value: device,
                  child: Text(
                    device.platformName.isNotEmpty
                        ? "${device.platformName} (${device.remoteId})"
                        : device.remoteId.toString(),
                    overflow: TextOverflow.ellipsis,
                  ),
                );
              }).toList(),
              onChanged: (value) => setState(() => selectedDevice = value),
            ),
            const SizedBox(height: 10),
            ElevatedButton(
              onPressed: _connectToDevice,
              style: ElevatedButton.styleFrom(
                backgroundColor: connectionState == BluetoothConnectionState.connected
                    ? Colors.green
                    : Colors.blue,
              ),
              child: Text(
                connectionState == BluetoothConnectionState.connected
                    ? 'CONNECTED'
                    : 'CONNECT DEVICE',
                style: const TextStyle(color: Colors.white),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildWifiForm() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(12.0),
        child: Column(
          children: [
            TextFormField(
              controller: ssidController,
              decoration: const InputDecoration(
                labelText: 'WiFi SSID',
                icon: Icon(Icons.wifi),
              ),
            ),
            TextFormField(
              controller: passwordController,
              decoration: const InputDecoration(
                labelText: 'WiFi Password',
                icon: Icon(Icons.lock),
              ),
              obscureText: true,
            ),
            const SizedBox(height: 10),
            ElevatedButton(
              onPressed: _sendWifiCredentials,
              child: const Text('SEND CREDENTIALS'),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildLocationInfo() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(12.0),
        child: Column(
          children: [
            const Text("GPS STATUS", style: TextStyle(fontWeight: FontWeight.bold)),
            const SizedBox(height: 10),
            currentPosition != null
                ? Text(
              'Lat: ${NumberFormat('#.#####').format(currentPosition!.latitude)}\n'
                  'Lon: ${NumberFormat('#.#####').format(currentPosition!.longitude)}',
              textAlign: TextAlign.center,
            )
                : const Text("No location data", style: TextStyle(color: Colors.grey)),
          ],
        ),
      ),
    );
  }

  @override
  void dispose() {
    _connectionStateSubscription?.cancel();
    scanSubscription?.cancel();
    FlutterBluePlus.stopScan();
    positionStream?.drain();
    ssidController.dispose();
    passwordController.dispose();
    super.dispose();
  }
}