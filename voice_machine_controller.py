#　ArduinoからのCdsセンサ出力受信してPythonで読む
#　PythonからArduinoに命令を送る
''' 参考 
[1] https://next-k.site/blog/archives/2021/12/09/645
[2] https://codechacha.com/ja/python-convert-string-to-bytes/
'''
import serial
import time
import threading

ser = serial.Serial("COM3",115200) #Arduinoのシリアルポート番号と伝送速度を設定

def func_send():
    while True:
        contorol_command=input("Enter the control command: ")
        print("send1")
        sent_command="{0};".format(contorol_command)
        byte_sent_command=sent_command.encode('utf-8')
        ser.write(byte_sent_command) #制御命令を送信
        time.sleep(1)   # 1秒間隔で制御命令を送信
        # set.write(send_data.encode('utf-8'))

def func_read():
    while True:
        time.sleep(0.1) # 10[ms]間隔で計測データを読み取り
        # print("read")
        val_arduino=ser.readline()
        val_decoded=val_arduino.strip().decode("UTF-8")
        print(val_decoded) #val_arduino:byte型なのでb'文字列'となって出てくる

if __name__=="__main__":
    thread_1=threading.Thread(target=func_send)
    thread_2=threading.Thread(target=func_read)

    thread_1.start()
    thread_2.start()

# ser.close()