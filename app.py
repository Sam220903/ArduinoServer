# app.py (Servidor Flask)
from flask import Flask, request, jsonify
import pyodbc

app = Flask(__name__)

# Cadena de conexión para SQL Server en Azure
conn = pyodbc.connect("DRIVER={ODBC Driver 17 for SQL Server};"
                      "SERVER="
                      "DATABASE=;"
                      "UID=;"
                      "PWD=;")

@app.route('/datos', methods=['POST'])
def recibir_datos():
    data = request.get_json()

    # Extraer los valores de los datos JSON
    temperature = data['temperature_sensor']
    humidity = data['humidity_sensor']
    soil_moisture = data['sm_sensor']
    light = data['light_sensor']
    ph_level = data['ph_sensor']

    # Crear el cursor e insertar los datos en la base de datos
    cursor = conn.cursor()
    
    cursor.executemany('''
    INSERT INTO sa_datas (idsa_sensor, stats) 
    VALUES (?, ?)''', 
    [(1, soil_moisture), (2, humidity), (3, light), (4, ph_level), (5, temperature)]
    )


    conn.commit()
    cursor.close()

    return jsonify({"status": "Datos recibidos y almacenados con éxito"}), 201

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=80)
