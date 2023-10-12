import json
from functools import reduce
import math
from operator import add
import random
from flask import Flask, request, jsonify, render_template, redirect, url_for
import smtplib, ssl
import sqlite3, re

app = Flask(__name__)

# Default basket height
actual_basket_height = 80.00 # unit: cm
default_basket_height = actual_basket_height * 0.95 

default_recommend_temp = 0 # default: 0 for cold


@app.route('/user-email.html')
def displayUserEmailForm():
    return render_template("user-email.html")

@app.route('/wifimanager.html')
def displayWifimanager():
    return render_template("wifimanager.html")

@app.route('/sendEmail', methods=['POST'])
def sendEmail():
    # sample: {"message" : "This is a sample email message"}
    message = json.loads(request.data)["message"] 
    # print(message)
    port = 587  # For starttls
    smtp_server = "smtp.gmail.com"
    sender_email = "sduri.pro@gmail.com"
    receiver_email = "sduri.pro@gmail.com"
    password = "ymwqjfefcsnfgzri" # input("Type your password and press enter:")

    context = ssl.create_default_context()
    with smtplib.SMTP(smtp_server, port) as server:
        
        server.ehlo()  # Can be omitted
        server.starttls(context=context)
        server.ehlo()  # Can be omitted
        server.login(sender_email, password)
        server.sendmail(sender_email, receiver_email, message)
    print("message : Email sent successfully!")
    print("Sent email : ", message)
    return jsonify({"message": "Email sent successfully!"})

# @app.route('/data', methods=['POST'])
# def receive_data():
#     data = request.json
#     print("Received data:", data)

#     # Update the SQLite database
#     with sqlite3.connect('clothes_db.sqlite') as conn:
#         cursor = conn.cursor()
#         cursor.execute("""
#                 INSERT INTO Clothes (unique_id, clothes_type_id, washing_temperature_id, sensor_distance)
#                 VALUES (?, ?, ?, ?)
#                 """, (data['unique_id'], data['clothes_type_id'], data['washing_temperature_id'], data['sensor_distance']))
        
#         rows_affected = cursor.rowcount
#         print(f"Rows affected by update: {rows_affected}")
#         conn.commit()

#     if rows_affected == 0:
#         return jsonify({"error": f"No rows with unique_id {data['unique_id']} found!"}), 400

#     return jsonify({"message": "Data updated successfully!"})



@app.route('/sensor', methods=['POST'])
def sensor_endpoint():
    # Directly read the raw data from the request
    sensor_value_text = request.data.decode('utf-8')
    
    # Print the received value
    print(f"Received sensor value: {sensor_value_text, type(sensor_value_text)}")

    if math.ceil(float(sensor_value_text)) > default_basket_height or math.ceil(float(sensor_value_text)) <= 0:
        return jsonify({"message": "Error sensor reading! Database is NOT updated!"})
    
    # If you also want to use it as a float for further processing, you can convert it:
    try:
        sensor_value = float(sensor_value_text)
    except ValueError:
        return "not float"
    
    with sqlite3.connect('washing_db_corrected.sqlite') as conn:
        # print("type(sensor_value): ", type(sensor_value))
        cursor = conn.cursor()
        cursor.execute('''INSERT INTO DistanceState (distance) VALUES (?);''', (sensor_value,))
        conn.commit()

    rows_affected = cursor.rowcount
    print(f"Rows affected by update: {rows_affected}")
        
    if rows_affected == 0:  
        return jsonify({"error": f"error nothing happened"}), 400
    
    print("message: Sensor Reading Data updated successfully!")
    # return redirect(url_for('home', distance=math.ceil(float(sensor_value)))) 
    return jsonify({"message": "Sensor Reading Data updated successfully!"})

@app.route('/rfid', methods=['POST'])
def rfid_endpoint():
    data = request.json
    print(data)
    tag_id = data['tag_id']
    cloth_type = data['tag_content'].split("-")[1]
    cloth_color = data['tag_content'].split("-")[2]
    # print(cloth_type, cloth_color)
    distance = 0 # can discuss later
    
    with sqlite3.connect('washing_db_corrected.sqlite') as conn:
        cursor = conn.cursor()
        
        cursor.execute('''INSERT INTO TempInput (tag, cloth_type, distance, cloth_color) VALUES (?,?,?,?)''', 
                       (tag_id, cloth_type.capitalize(), distance, cloth_color.capitalize()))
        conn.commit()

        rows_affected = cursor.rowcount
        print(f"Rows affected by update: {rows_affected}")
        
    if rows_affected == 0:  
        return jsonify({"error": f"error nothing happened"}), 400
    
    # For now, just return a stubbed response
    return jsonify({"message": "RFID data processed!"})

def get_table_from_db(table_names, colnames=None):
    """
    Returns all rows from a table as a list of lists
    """
    
    desired_tables = []
    with sqlite3.connect('washing_db_corrected.sqlite') as conn:
        cursor = conn.cursor()

        if type(table_names) is not list:
            table_names = re.split(',|_|-|!', table_names.replace(" ",""))

        if type(colnames) is not None and type(colnames) is  list:
            colnames = re.split(',|_|-|!', colnames.replace(" ",""))
               
        for table_name in table_names:
            # print(table_name)
            
            if colnames is None:
                query = 'SELECT * FROM ' + table_name
                desired_tables.append(conn.execute(query).fetchall())
            else:
                query = 'SELECT ' + ''.join(colnames) + ' FROM ' + table_name
                # print("query: " + query)
                desired_tables.append(conn.execute(query).fetchall())

    # print("\ndesired_tables: \n", desired_tables)
    if len(table_names) == 1:
        return reduce(add, desired_tables)
    else:
        return desired_tables

@app.route('/get_distance_val', methods=['POST', 'GET'])
def get_distance_val():
    # print("distance: " + str(get_table_from_db("DistanceState")[-1][1]))
    return str(get_table_from_db("DistanceState")[-1][1])
    # return str(random.randint(10,80)) # can be used for testing

@app.route('/get_temp_val', methods=['POST', 'GET'])
def get_temperature_val():
    recommended_temp = get_table_from_db("Clothes", "washing_temperature_id")
    #removing all non-type 
    recommended_temp = [sub for sub in recommended_temp if not all(ele == None for ele in sub)]
    recommended_temp = str(max(recommended_temp)[0])
    recommended_temp = recommended_temp if  recommended_temp is not None else default_recommend_temp
    print("recommended_temp: ", recommended_temp, type(recommended_temp))
    return recommended_temp

@app.route('/content.html')
def display_clothes():
    
    with sqlite3.connect('washing_db_corrected.sqlite') as conn:
        cursor = conn.cursor()
        clothes = get_table_from_db("Clothes") # conn.execute('SELECT * FROM Clothes').fetchall()
        colors = dict(get_table_from_db("ClothesColor")) # dict(conn.execute('SELECT * FROM ClothesColor').fetchall())
        clothe_types = get_table_from_db("ClothType") #conn.execute('SELECT id, name FROM ClothType').fetchall()
        clothe_types = dict([(i[0], i[1]) for i in clothe_types])
        temp = dict(get_table_from_db("Temperature") ) # dict(conn.execute('SELECT * FROM Temperature').fetchall())


        print("clothes: ", clothes)
        list_to_display = []

        for cloth in clothes:
            temp_list = []
            # print(int(cloth[1]))
            # print(type(int(cloth[1]))) # str
            temp_list.append(clothe_types[int(cloth[1])]) # cloth_type_id
            if cloth[-1] is not None:
                temp_list.append(colors[cloth[-1]]) # cloth_color_id
                # print(colors[cloth[-1]])
            else:
                # print("Unknown")
                temp_list.append("Unknown")
            # temp_list.append(cloth[2])
            # print(colors[i[-1]])
            # print(colors[list(i)[-1]])

            list_to_display.append(temp_list)
        # print(list_to_display)
    return render_template('/content.html', data=list_to_display)



@app.route('/',  methods=['GET', 'POST'])
@app.route('/<distance>', methods=['GET', 'POST'])
@app.route('/index.html') 
def home(distance=None): 

    

    distance_from_args = distance # request.args.get("distance")
    # print("distance_from_args: ", distance_from_args)
    distance = distance_from_args if distance_from_args is not None else get_table_from_db("DistanceState")[-1][1]
    # print("distance_from_args::distance: ", distance)

    if int(distance) <= 0.15 * default_basket_height:
        distance = 0.15 * default_basket_height
    return render_template('index.html', 
                          
                          ########## start: To be used only for Testing ########################
                           #uncomment below line for testing "Oops! ..." email and "Reminder! ..." email
                        #    distance_val = 72, 
                           #for testing "Reminder! ..." email: set distance_val= 72 
                           # and set recommended_temp = 2 in script.js file 
                           ########## end: To be used only for Testing ########################

                           distance_val= distance, 
                           defaultBasketLength=default_basket_height
                           )

if __name__ == "__main__":
    # app.run(host='0.0.0.0', port=5003)
    # app.run(host='localhost', port=5003, debug=True)
	# app.run(host='localhost', port=5003, debug=True, threaded=True)
    app.run(host='192.168.1.101', port=5003, debug=True, threaded=True)
    # app.run(host='0.0.0.0', port=5003, debug=True, threaded=True)