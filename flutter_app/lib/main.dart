import 'dart:async';
import 'dart:io';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:geolocator/geolocator.dart';
import 'package:intl/intl.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:flutter/services.dart';

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

  @override
  void initState() {
    super.initState();
    _checkPermissions();
  }

  Future<void> _checkPermissions() async {
    // 请求位置权限
    if (Platform.isAndroid || Platform.isIOS) {
      Map<Permission, PermissionStatus> statuses = await [
        Permission.location,
        Permission.bluetooth,
        Permission.bluetoothAdvertise,
        Permission.bluetoothConnect,
        Permission.bluetoothScan,
      ].request();

      print("权限状态:");
      statuses.forEach((key, value) => print("${key}: $value"));
    }
  }

  Future<void> _scanDevices() async {
    // 检查蓝牙是否可用
    if (await FlutterBluePlus.isSupported == false) {
      print("Bluetooth not available");
      return;
    }

    // 停止现有扫描
    if (FlutterBluePlus.isScanningNow) {
      await FlutterBluePlus.stopScan();
      print("Stopped previous scan");
    }

    devices.clear();
    setState(() {}); // 立即清空列表

    // 开始扫描
    print("Starting BLE scan...");
    scanSubscription = FlutterBluePlus.scanResults.listen((results) {
      for (ScanResult result in results) {
        if (!devices.any((d) => d.remoteId == result.device.remoteId)) {
          print("Found device: ${result.device.remoteId} | RSSI: ${result.rssi}");
          setState(() {
            devices.add(result.device);
          });
        }
      }
    }, onError: (e) => print("Scan error: $e"));

    await FlutterBluePlus.startScan(
      timeout: const Duration(seconds: 10),
      androidUsesFineLocation: true,
    );
  }

  Future<void> _connectToDevice() async {
    if (selectedDevice == null) return;

    print("Connecting to ${selectedDevice!.remoteId}");
    try {
      await selectedDevice!.connect();
      setState(() {
        connectionState = BluetoothConnectionState.connected;
      });
      _startLocationUpdates();
    } catch (e) {
      print("Connection failed: $e");
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
      print("New position: $position");
      setState(() {
        currentPosition = position;
      });
      _sendLocationData();
    });
  }

  Future<void> _sendLocationData() async {
    if (selectedDevice == null || currentPosition == null) return;

    try {
      List<BluetoothService> services = await selectedDevice!.discoverServices();

      for (BluetoothService service in services) {
        if (service.uuid.toString() == '0000ff01-0000-1000-8000-00805f9b34fb') {
          for (BluetoothCharacteristic characteristic in service.characteristics) {
            if (characteristic.uuid.toString() == '0000ff02-0000-1000-8000-00805f9b34fb') {
              String locationData =
                  'LAT:${currentPosition!.latitude},LON:${currentPosition!.longitude}';
              await characteristic.write(locationData.codeUnits);
              print("Sent location data: $locationData");
            }
          }
        }
      }
    } catch (e) {
      print("Error sending location: $e");
    }
  }

  Future<void> _sendWifiCredentials() async {
    if (selectedDevice == null) return;

    try {
      List<BluetoothService> services = await selectedDevice!.discoverServices();

      for (BluetoothService service in services) {
        if (service.uuid.toString() == '0000ff01-0000-1000-8000-00805f9b34fb') {
          for (BluetoothCharacteristic characteristic in service.characteristics) {
            if (characteristic.uuid.toString() == '0000ff03-0000-1000-8000-00805f9b34fb') {
              String credentials =
                  'SSID:${ssidController.text},PASS:${passwordController.text}';
              await characteristic.write(credentials.codeUnits);
              print("Sent WiFi credentials: $credentials");
            }
          }
        }
      }
    } catch (e) {
      print("Error sending credentials: $e");
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
            onPressed: () => _checkPermissions(),
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
    return Column(
      children: [
        ElevatedButton(
          onPressed: _scanDevices,
          child: const Text('Scan Devices'),
        ),
        const SizedBox(height: 10),
        Text("Found ${devices.length} devices",
            style: const TextStyle(color: Colors.grey)),
      ],
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
              child: const Text('SYNC CREDENTIALS'),
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
            const Text("GPS Status",
                style: TextStyle(fontWeight: FontWeight.bold)),
            const SizedBox(height: 10),
            currentPosition != null
                ? Text(
              'Lat: ${NumberFormat('#.#####').format(currentPosition!.latitude)}\n'
                  'Lon: ${NumberFormat('#.#####').format(currentPosition!.longitude)}',
              textAlign: TextAlign.center,
            )
                : const Text("No location data",
                style: TextStyle(color: Colors.grey)),
          ],
        ),
      ),
    );
  }

  @override
  void dispose() {
    scanSubscription?.cancel();
    FlutterBluePlus.stopScan();
    positionStream?.drain();
    ssidController.dispose();
    passwordController.dispose();
    super.dispose();
  }
}