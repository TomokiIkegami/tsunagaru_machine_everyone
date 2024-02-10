# This is a lang controlled program.
#　ArduinoからのCdsセンサ出力受信してPythonで読む
#　PythonからArduinoに命令を送る
''' 参考 
[1] https://next-k.site/blog/archives/2021/12/09/645
[2] https://codechacha.com/ja/python-convert-string-to-bytes/
[3] https://qiita.com/umi_mori/items/757834e0ef75f38cea19
'''
# import serial   # シリアル通信ライブラリ]
import cv2   # OpenCVライブラリ
import time    # 時間関連のライブラリ
import threading    # マルチスレッド処理ライブラリ
import sys   # システム関連のライブラリ

import lang # 翻訳用プログラム
# import hand # 手の動き検出用プログラム
from hand import HandDirectionDetector
# ser = serial.Serial("COM6",115200) HandDirectionDetector#Arduinoのシリアルポート番号と伝送速度を設定

def func_send():
    while True:
        # Your code here
        result_direction = hand_detector.detect_hand_direction(frame)
        if result_direction is not None:
            print(f'Speed: {result_direction}')

    # while True:   # キーボード入力によって、Arduinoに制御命令を送信
    #     contorol_command=hand.HandDirectionDetector.detect_hand_direction()
    #     print("Detected: ",contorol_command)
    #     sent_command="{0};".format(contorol_command)
    #     byte_sent_command=sent_command.encode('utf-8')
    #     if(contorol_command=="q"):  # qを入力するとプログラムを終了
    #         ser.close()
    #         sys.exit(0)
    #     ser.write(byte_sent_command) #制御命令を送信
    #     time.sleep(1)   # 1秒間隔で制御命令を送信
        # set.write(send_data.encode('utf-8'))

def func_read():
    while True:
        time.sleep(0.1) # 10[ms]間隔で計測データを読み取り
        print("read")
        # val_arduino=ser.readline()
        # val_decoded=val_arduino.strip().decode("UTF-8")
        # print(val_decoded) #val_arduino:byte型なのでb'文字列'となって出てくる

if __name__=="__main__":
    thread_1=threading.Thread(target=func_send)
    thread_2=threading.Thread(target=func_read)

    thread_1.start()
    thread_2.start()

    # Inisialisasi objek HandDirectionDetector
# hand_detector = HandDirectionDetector()

    hand_detector = HandDirectionDetector()
# Inisialisasi tangkapan video
    video = cv2.VideoCapture(0)

    # Loop utama
    while True:
        # Membaca frame dari video
        ret, frame = video.read()
        frame = cv2.flip(frame, 1)

        # Mendapatkan arah dari objek HandDirectionDetector
        result_direction = hand_detector.detect_hand_direction(frame)

        # Print hasil di terminal
        if result_direction is not None:
            print(f'Speed: {result_direction}')

        # Menampilkan frame
        cv2.imshow("frame", frame)

        # Cek jika tombol 'k' ditekan untuk keluar dari loop
        k = cv2.waitKey(1)
        if k == ord("k"):
            break

    # Melepas tangkapan video dan menutup jendela OpenCV
    video.release()
    cv2.destroyAllWindows

    # ser.close()
        