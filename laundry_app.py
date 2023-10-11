from flask import Flask, request, jsonify, render_template
import sqlite3

app = Flask(__name__)

# Default sensor distance
default_sensor_distance = 45.00  # in cm

@app.route('/sensor_test', methods=['POST'])
def posttome():
    print(request.data)
    return 'vum'

@app.route('/data', methods=['POST'])
def receive_data():
    data = request.json
    print("Received data:", data)

    # Update the SQLite database
    with sqlite3.connect('clothes_db.sqlite') as conn:
        cursor = conn.cursor()
        cursor.execute("""
                INSERT INTO Clothes (unique_id, clothes_type_id, washing_temperature_id, sensor_distance)
                VALUES (?, ?, ?, ?)
                """, (data['unique_id'], data['clothes_type_id'], data['washing_temperature_id'], data['sensor_distance']))
        
        rows_affected = cursor.rowcount
        print(f"Rows affected by update: {rows_affected}")
        conn.commit()

    if rows_affected == 0:
        return jsonify({"error": f"No rows with unique_id {data['unique_id']} found!"}), 400

    return jsonify({"message": "Data updated successfully!"})

@app.route('/test2', methods=['POST'])
def test2():
    print("post test")
    # return "index.html"

@app.route('/sensor', methods=['POST'])
def sensor_endpoint():
    # Directly read the raw data from the request
    sensor_value_text = request.data.decode('utf-8')
    
    # Print the received value
    print(f"Received sensor value: {sensor_value_text}")

    # If you also want to use it as a float for further processing, you can convert it:
    try:
        sensor_value = float(sensor_value_text)
    except ValueError:
        return "not float"
    
    with sqlite3.connect('clothes_db.sqlite') as conn:
        cursor = conn.cursor()
        cursor.execute('INSERT INTO "Sensor Data" (sensor_reading) VALUES (?)', (sensor_value,))
        conn.commit()

        rows_affected = cursor.rowcount
        print(f"Rows affected by update: {rows_affected}")
        
    if rows_affected == 0:  
        return jsonify({"error": f"error nothing happened"}), 400
    
    return jsonify({"message": "Sensor Reading Data updated successfully!"})
    


@app.route('/rfid', methods=['POST'])
def rfid_endpoint():
    data = request.json
    print(data)
    
    # For now, just return a stubbed response
    return jsonify({"message": "RFID data processed!"})

def differentiate_clothes(tag_info):
    # Placeholder function for processing RFID tag information
    pass

@app.route('/vum', methods=['GET'])
def get_data():
    data = request.json
    print("Getted data:", data)

@app.route('/test') 
def test(): 
    # return 'Hello, World! this application runing on 192.168.0.105'
    return "hello"

@app.route('/content.html')
def display_clothes():
    
    with sqlite3.connect('washing_db_corrected.sqlite') as conn:
        cursor = conn.cursor()
        # cursor.execute("""
                # SELECT * FROM TempInput
                # """)
        clothes = conn.execute('SELECT * FROM Clothes').fetchall()
        colors = dict(conn.execute('SELECT * FROM ClothesColor').fetchall())
        clothe_types = conn.execute('SELECT id, name FROM ClothType').fetchall()
        clothe_types = dict([(i[0], i[1]) for i in clothe_types])
        temp = dict(conn.execute('SELECT * FROM Temperature').fetchall())
        print(type(clothe_types))
        print(clothe_types)

        list_to_display = []
        
        for cloth in clothes:
            temp_list = []
            print(type(int(cloth[1]))) # str
            temp_list.append(clothe_types[int(cloth[1])])
            if cloth[-1] is not None:
                temp_list.append(colors[cloth[-1]])
                # print(colors[cloth[-1]])
            else:
                # print("Unknown")
                temp_list.append("Unknown")
            # temp_list.append(cloth[2])
            # print(colors[i[-1]])
            # print(colors[list(i)[-1]])
        # # list_to_display = 
            list_to_display.append(temp_list)
        print(list_to_display)
    return render_template('/content.html', data=list_to_display)

@app.route('/') 
def home(): 
    # return 'Hello, World! this application runing on 192.168.0.105'
    return render_template('index.html', value=10)


if __name__ == "__main__":
    # app.run(host='0.0.0.0', port=5003)
    # app.run(host='localhost', port=5003, debug=True)
	app.run(host='localhost', port=5003, debug=True, threaded=True)