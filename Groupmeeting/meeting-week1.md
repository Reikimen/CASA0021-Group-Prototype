---
typora-root-url: ../.
---

## Meeting Information
- **Topic**:  Popotype Design
- **Date**:  27/1/2025
- **Location**:  UCL EAST OPS
- **Attendees**:  Everyone in CASA0021 Group 2
- **Note-taker**:  Dankao

---

## Agenda
1.  Shape design and user interaction
1.  Idea of "linked"
1.  Purchasing list

---

## Discussion Notes

### 1. Topic 1: Shape design and user interaction
- **Discussion**:  

  - **UX Idea 1:** Cosmetic mirror opening and closing

    <img src="/Img/Groupmeeting/week1/WhatsApp Image 2025-01-30 at 11.27.02.jpeg" alt="WhatsApp Image 2025-01-30 at 11.27.02" style="zoom:20%;" />

  - **UX Idea 2:** The side is attached to the LED light belt, through which the direction is indicated.

    <img src="/Img/Groupmeeting/week1/WhatsApp Image 2025-01-30 at 11.28.22.jpeg" alt="WhatsApp Image 2025-01-30 at 11.28.22" style="zoom:20%;" />
    
  - **UX Idea 3:** On the basis of Idea2, the fixed pointer is changed to the luminous pointer
  
    <img src="/Img/Groupmeeting/week1/WhatsApp Image 2025-01-30 at 11.27.58.jpeg" alt="WhatsApp Image 2025-01-30 at 11.27.58" style="zoom:25%;" />

- **Decisions/Actions**:  UX Idea 3 was chose.

- **Deadline**:  /

### 2. Topic 2: Idea of "linked"
- **Discussion**: 

  - **Linked Idea 1:** 

    The original idea is - Connected to each other in a physical sense, connected when face to face. Therefore GPS had to be used to locate the devices to ensure a face-to-face connection.

    **Issues:**

    1. GPS module consumes more energy
    2. GPS modules are expensive, resulting in higher equipment production costs

    **Solutions:**

    1. Each time the device is switched on, the GPS is used to obtain location information (for about 10s), and is switched off as soon as the location information is obtained, unless the user performs a repositioning operation while using the device, in which case the GPS is switched on for a short period of time, obtains the information, and is switched off automatically.
    2. The Second solution is to use a mobile app that retrieves location details from the personal smartphone of the user and send it to the device to be used in the calculations. This would remove the need for a built-in GPS which automaticaly solves the problem of powering and cost of implementing a GPS module within the device.

  - **Linked Idea 2:** 

    Suppose there are two users, Jack and Rose, who are lovers in a long-distance relationship. Suppose Rose, at a certain point in time, feels sad because her job is not going well. So she switches on our compass, and after switching on the device, she rotates it at a particular angle relative to when the device was switched on and presses the ‘Stressed Out’ button, while Jack receives a vibration from the compass at the same time 📳. Jack, after switching on the device, rotates the angle at which Rose pressed the button, at which point the light band on the compass flashes and vibrates slightly, picking up Rose's mood message.
    
    <img src="/Img/Groupmeeting/week1/WhatsApp Image 2025-01-30 at 11.37.17.jpeg" alt="WhatsApp Image 2025-01-30 at 11.37.17" style="zoom:25%;" />

- **Decisions/Actions**:  

  Not decided yet

- **Deadline**:  /

### 3. Topic 3: Purchasing list
- **Discussion**:  According to Idea 3

- **Decisions/Actions**: 

  Here is the Bom Chart

  | ID | Component      | Type       | Num | Link        |
  |------|----------------|---------------|------|------------|
  | 1    | Magnetometer Sensor| HMC5883L | 3    | [HMC5883L](https://www.amazon.co.uk/DAOKAI-Three-axis-magnetic-QMC5883L-Magnetometer/dp/B0CD77DN87/ref=sr_1_3?crid=3OSK5IIR2HEGC&dib=eyJ2IjoiMSJ9.zhjEjRaO1IloqwnSW6UebLLmc6Dsp2pbyqGbSr8xB-eFEaw-sSWzjv-wwkbZ1PRiLfKX4ir78RFcUDraV58lyoBZV0ykpmuABqXYrPdK1BqrWV3N68fOd0T-JvMsDN1BoSQcluNCwzxTncvPa1DfaRzVjEIfV-4DW72nh_nqGr1Lj9QkKUzEOyN1ylN2WGZwy-7WOFmufrSJZNQfiFFwsq9l0lYZiZjPVDwOU4XekU10lNxOCqvVe_FQTDFnhU2h44tIoXRIQuD6kxB9-dCUmtox2xgCbHfv8WhvHZxvzwfkDT0C1Nex-uO5d6KfXnInrrFvpE0lE6sehvz71qWcjSsYoM84x0h_eEvWZVJAlAAad5GA7BZVpTj02gIqLXmOrOB-Znnk_kkKs2c-hSeFr52Mbe9Nqkr53PXVI0ip0rlawLrKgDWA7muQnh1dsHGB.ym47vcvC8hKjixKSOB8uIgmPTslufP1PgO6kdFOd590&dib_tag=se&keywords=HMC5883L&qid=1738254153&sprefix=hmc5883l%2Caps%2C99&sr=8-3)     |
  | 2    | Push Button    | 12pcs 12mm     | 1    |[Push Button](https://www.amazon.co.uk/gp/product/B07XQSBW7Q/ref=sw_img_1?smid=A1DBC97EH2O973&th=1)      |
  | 3    | 3PCS Vibration Motor | M20 Mini | 1   | [M20 Mini](https://www.amazon.co.uk/Vibration-Electric-Vibrating-Eccentric-Instrument/dp/B07WP2PGBW/ref=sr_1_31?crid=1SULMIOIENY2&dib=eyJ2IjoiMSJ9.aOjDBhvfppVB_Xq_Y4Mug5dlS7q1JZpz_t7I4qh8zfwMwTkJqLccgG0i8QBZQ8Hdq0-0injkcuLgFlf7qwjGBzrjmMAYssYgN4i9nwR-SsvgDjvDlIxLEmGajZHSP0cLsN9G0xfCTdEh69QXZiH7bsMEUB-PaMxtp2nKdAu2wwBUryggSCvvlLLGXyEVpFWnQcDwE8L7N9R3BXjlw4n2MzIl5xHvCRJvZm6H-RPpjQBV2VjL43hKdEDo803kCGwVPTJReRD5mLPeTpjdv51Fn-WZqUUDXg9cf8MStMCB-xC0ZTQ4wk6WTqChagiRnyfIA2jTAN6mb6Ko1nFZepblAqlFjuR-jiHkVn8zPpdlSwc.yUS1KBJ2eQcyx6BNh72EnsERR4vDJhmGcPpQmuXGRdE&dib_tag=se&keywords=vibration+motor&qid=1738254932&s=industrial&sprefix=vibration+motor%2Cindustrial%2C85&sr=1-31) |
  | 4    | 2PCS GPS Module | Aideepen | 1    | [GPS Module](https://www.amazon.co.uk/Aideepen-GY-GPS6MV2-Position-Antenna-Controller/dp/B08CZSL193/ref=sr_1_4?crid=2VW400T6XUZQO&dib=eyJ2IjoiMSJ9.IQEnDal4ccImPnN2M5hPECmBuR3YWwWJuU73rkriFdwMvxpTcHblIdbeItblJiL4TA74Mh6Y0ZWMGQOQkzjhR7KbecqomQI3tSlBvmndVHwGHhxT-60h0F7vBQ604MqymzhWg2VY-qrgjQnREQ3la77QYjtUAi8eu0TkqYqqzch_K3gNif7SllqGP4BNy5SDMQsQZ-tzvOp4KW6-v2G9FIzgkJ6l8-ZtpaXWkWFrNus.-AmpJyZaqqUKqqstSgVGqnDzWkU5HIOdoliNiOieHP4&dib_tag=se&keywords=gps+module&qid=1738255200&sprefix=GPS+%2Caps%2C80&sr=8-4)   |

  Subtotal (6 items): **£53.03**


- **Deadline**: /

---

## Action Items
| Task Description                     | Owner   | Deadline  | Status        |
| ------------------------------------ | ------- | --------- | ------------- |
| Shape design and user interaction    | Group 6 | /         | On going      |
| Purchasing items                     | Dankao  | 30/1/2025 | On going      |
| Decision of Linking Idea (ask tutor) | Group 6 | /         | Not Start Yet |


---

## Other Notes (left Questions)
-  Decision of Linking Idea
-  The manner in which a user enters information with the device and how much information is exchanged with another user
