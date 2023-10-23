# Internet of Things (CITS5506) team project
## Project name
Capacity - Monitoring smart Laundry Basket with NFC Tag Detection 
## Project description (Abstract)
Managing normal tasks like doing laundry can be overlooked in today's fast-paced world, especially for 
those with packed schedules. Our project introduces a state-of-the-art smart laundry basket equipped 
with NFC tag detection and capacity monitoring to address this issue. Through database management, 
we associate the capacity of the laundry basket with a specific distance state and the temperature of 
the laundry water with different types of clothing. This enables the system to update the status of the 
laundry basket in real time based on the detected distance to provide the user's washing requirements. 
In addition, when the system scans the RFID tags on the user's laundry, it notifies the user of the current 
laundry state in the basket and the washing recommendations. The core of the system relies on an 
ESP32 microcontroller equipped with an NFC component, an LED indicator, and a distance sensor, all 
powered by a high-efficiency battery pack. In addition to notifications, this data is processed and 
synchronized to a cloud server, allowing users to access detailed laundry information through a web 
interface. This seamless integration of technology into everyday household tasks not only ensures 
optimization of laundry routines but also improves energy efficiency and extends the life of clothing. As 
we continue to push the boundaries of home management, we hope that in the future, more technology 
will be integrated with everyday tasks to promote efficiency and sustainability.

### Getting Started
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Installing
This application can be deployed locally, install git and clone the reposistory.

### Get the code:
```
git clone https://github.com/VumGualnam/iotbased_Monitoring-smart-Laundry-Basket-with-NFC-Tag-Detection.git
```

### Generate a virtual environment and install dependencies:
```
Unix:
$ python3 -m venv venv
$ source venv/bin/activate
(venv) $ python3 -m pip install --upgrade pip
(venv) $ pip install -r requirements.txt

Windows:
$ python3 -m venv venv
$ venv\Scripts\activate.bat
(venv) $ python3 -m pip install --upgrade pip
(venv) $ pip install -r requirements.txt
```

### Running
cd into ***laundry_app.py*** directory, then
```
$ python laundry_app.py
```

### Authors
FirstName_Initial Surname_Initial Email

* A V (22791631@student.uwa.edu.au)
* L C (23623238@student.uwa.edu.au)
* P W (23750564@student.uwa.edu.au)
* V G (22355071@student.uwa.edu.au)
