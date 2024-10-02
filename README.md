# Envio de datos de arduino a DB en Azure
El presente conjunto de códigos es un compendio de la lógica necesaria para recuperar los datos de un circuito de sensores controlado con Aarduino y enviarlos a una base de datos alojada en Azure

### Código arduino
Para ejecutar el código arduino, debe contar con un sensor controlado por una tarjeta Arduino MKR 1010 WiFi, copiar y pegar el código presnetado en un nuevo skecth en Arduino IDE y subirlo a su circuito

### App.py
Para ejecutar la app desarrollada en Python - Flask, debe seguir los siguientes pasos.

* Crear un entorno virtual de Python
´´´
bash
python -m venv .venv
´´´

* Activar el entorno virtual
´´´
bash
.venv/Scripts/activate
´´´

* Instalar los requerimientos
´´´
bash
pip install -r requirements.txt
´´´