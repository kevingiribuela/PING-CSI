# _Ping y CSI_
Este software ha sido desarrollado para poder obtener el CSI (Channel State Information) entre un ESP32 y un router utilizando diferentes protocolos de comunicación y canales disponibles en la banda de 2.4GHz.
El software está diseñado específicamente para una placa diseñada con KiCAD cuya documentación se encuentra en el siguiente enlace:

Sin embargo, es posible utilizar el mismo software cambiando la definición de 2 pines, el pulsador de propósito general, y el LED. Es muy sencillo dado que se encuentran en las primeras líneas de código del main.c

## ¿Cómo utilizarlo?
A continuación se describe la utilización del código para el caso de que se utilice la placa diseñada en KiCAD V6. Se asume que el usuario ha grabado el software en la placa y ya tiene el dispositivo listo para ser configurado.
1) La placa comenzará a destellar el LED con un período de 300mS indicando que se encuentra en modo AP. En esta situación, el ESP32 genera un webserver en el cuál se le pasa a través de él los datos de la red a la cual se desea que el dispositivo se conecte junto con el protocolo de comunicación a utilizar, a saber 802.11b/bg/bgn.
2) Ingresar con un celular, tablet, notebook a la red "WIMUMO" con la clave "defiunlp2022".
3) Una vez conectado al dispositivo, se deberá ingresar a la siguiente dirección IP con cualquier navegador: 192.168.4.1
4) Dentro del webserver aparecerá la siguiente imagen 
### hola
## Example folder contents

 
