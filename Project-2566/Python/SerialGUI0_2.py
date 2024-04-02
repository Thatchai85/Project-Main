import serial
import json
import csv
import time
import threading
import random
import tkinter as tk
from pprint import pprint
import cv2

def open_camera():
    cv2.namedWindow("Preview")
    cam  = cv2.VideoCapture(1)
    if cam.isOpened():
        rval, frame = cam.read()
    else:
        rval = False

    while rval:
        cv2.imshow("Preview", frame)
        rval, frame = cam.read()
        key = cv2.waitKey(20)
        if key == 27:
            cv2.imwrite("Pic1.png", frame)
            print("image saved")
            break  
    cam.release()
    cv2.destroyAllWindows("Preview")    

def load_json(file_path):
    with open(file_path,'r') as file:
        data = json.load(file)
    return data

#Step calcualtion
def Steppercalculation(data):
    Values_x = []
    Values_y = []
    Process_time = [row[2] for row in data]
    #print("time:" ,Process_time)
    m2s = 0.2  # mm/step
    #Xfactor = 640/450 #px/mm.
    #Yfactor = 480/340 #px/mm.
    Xfactor = 356/257
    Yfactor = 362/265
    Xspace = 0
    Yspace = 20
    
    X_position = [row[0] for row in data]
    Y_position = [row[1] for row in data]
    
    for X_pixel, Y_pixel in zip(X_position,Y_position):
        X_Step = ((((640 - X_pixel) / Xfactor) / m2s) + Xspace)
        Y_Step = ((((480 - Y_pixel) / Yfactor) / m2s) + Yspace)
        
        #Values_x.append(round(X_Step,0))
        #Values_y.append(round(Y_Step,0))
        Values_x.append(int(X_Step))
        Values_y.append(int(Y_Step))
        
    return Values_x, Values_y, Process_time
 
#file_path_csv = "result-tray.csv"
#values_x = []
#values_y = []
#with open(file_path_csv, newline='') as csvfile:
#    data_csv = csv.reader(csvfile)
#    next(data_csv)  # ข้ามหัวตาราง
#    for row in data_csv:
#        values_x.append(int(row[0]))
#        values_y.append(int(row[1]))
# ตรวจสอบความถูกต้องของข้อมูลที่อ่านจากไฟล์ CSV
#print("Values X:", values_x)
#print("Values Y:", values_y)

# อ่านข้อมูลจากไฟล์ JSON และนำข้อมูล column ที่ 3 เข้าสู่ Process_time
#file_path_json = "data_18.json"
#with open(file_path_json) as json_file:
#    data_json = json.load(json_file)
#    Process_time = [item[2] for item in data_json['data']]

#print("Process_time:", Process_time)
#values_Stepx,values_Stepy = Steppercalculation(values_x,values_y)
file_path = "data_18.json"  # Adjust the path to your JSON file
data = load_json(file_path)
values_x, values_y, Process_time = Steppercalculation(data['data'])


class App(tk.Tk):
    def __init__(self, screenName=None, baseName=None, className="Tk", useTk=True, sync=False, use=None) -> None:
        super().__init__(screenName, baseName, className, useTk, sync, use)
        
        #values_x, values_y = Steppercalculation(self.data)
        #self.data = [(random.randint(0, 100), random.randint(0, 100), random.randint(1, 10)) for _ in range(18)]
        self.data = [(values_x[i], values_y[i], Process_time[i]) for i in range(18)]
        self.ser = serial.Serial('COM4', 115200, timeout=3)
        self.iterator = 0
        time.sleep(2)

        # Main frame
        self.title("Serial logging")
        self.Main_frame = tk.Frame(self)
        self.Main_frame.pack()

        self.Control_panel = tk.LabelFrame(self.Main_frame, text="Control center", relief="raised", font=('Arial'))
        self.Control_panel.grid(row=0, column=0, padx=20, pady=20, sticky='n')

        self.Start_button = tk.Button(self.Control_panel, text='START', command=self.start_event)
        self.Start_button.grid(row=0, column=0, padx=20, pady=20)
        self.Stop_button = tk.Button(self.Control_panel, text='laseron', command=self.Laseron_event)
        self.Stop_button.grid(row=0, column=1, padx=20, pady=20)
        self.Pause_button = tk.Button(self.Control_panel, text='Homeposition', command=self.Homeposition_event)
        self.Pause_button.grid(row=1, column=0, padx=20, pady=20)
        self.Continue_button = tk.Button(self.Control_panel, text='HomepositionY', command=self.HomepositionY_event)
        self.Continue_button.grid(row=1, column=1, padx=20, pady=20)

        thread1 = threading.Thread(target=self.receive_response)
        thread1.start()

    def send_package(self, command, data_list=None):
        package = {"command": command, "data": data_list}
        json_data = json.dumps(package)
        self.ser.write((json_data + '/n').encode('ascii'))
        self.ser.flush()
        print(f"Sent JSON package: {json_data}")
    
    def receive_response(self):
        data_pack = {"x": 0, "y": 0, "t": 0}
        try:
            while True:
                if self.ser.in_waiting > 0:
                    self.response = self.ser.readline().decode('utf-8').strip()
                    print(f"response : {self.response}")
                    if self.response == "request" and self.iterator != len(self.data):
                        print(self.iterator)
                        data_pack.update({"x":self.data[self.iterator][0], "y":self.data[self.iterator][1], "t":self.data[self.iterator][2]})
                        self.send_package("position", data_list=data_pack)
                        self.iterator += 1
                    elif self.response == "request" and self.iterator == len(self.data):
                        print("Final request received. Stopping")
                        #self.stop_event()
                        data_pack = {"x": 0, "y": 0, "t": 0}
                        self.send_package("position", data_list=data_pack)
                        break
        except Exception as e:
            print(f"Error in receive_response: {e}")
        except KeyboardInterrupt:
            pass        
                
    def start_event(self):
        for i in range(18):
            print("Point[",i+1,"]" ,"STEP[X]: ", round(values_x[i], 3), "\tSTEP[Y]: ", round(values_y[i], 3), "\ttime(seconds): ", Process_time[i])
        data_pack = {"x": 0, "y": 0, "t": 0}
        self.send_package("position", data_list=data_pack)

    def stop_event(self):
        #self.ser.flush()
        self.send_package(command="stop")
        data_pack = {"x": 0, "y": 0, "t": 0}
        self.send_package("position", data_list=data_pack)

    def Homeposition_event(self):
        self.send_package(command="Homeposition")
        # Open camera
        #open_camera()
        
    def HomepositionY_event(self):
        self.send_package(command="HomepositionY")

    def pause_event(self):
        self.send_package(command="pause")

    def Laseron_event(self):
        self.send_package(command="laseron")

    def continue_event(self):
        pass
    
if __name__ == '__main__':
    app = App()
    app.mainloop()
    
    