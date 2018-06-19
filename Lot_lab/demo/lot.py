import serial
import serial.tools.list_ports
import _thread

def init_serial():
    com = []
    port_list = list(serial.tools.list_ports.comports()) #读取设备的串口号
    for i in range(0, len(port_list)):
        com.append(list(port_list[i])[0])
    ser1 = serial.Serial(com[0], 115500)
    return ser1
#客厅执行函数
def Parlor_Choice_mode(command):
    global ser1
    print(command)
    ser1.write(command.encode())


#厨房执行函数
def Kitchen_Choice_mode(command):
    global ser1
    print(command)
    ser1.write(command.encode())

 #卧室执行函数
def Bedroom_Choice_mode(command):
    global ser1
    ser1.write(command.encode())
def Close_ALL():
    global ser1
    ser1.write('10'.encode())

def Rec_serial():
    global ser1

    global temperature #温度
    global humidity #湿度
    global illumination #光照
    global smoke #烟雾
    data=[]
    while True:
          str_data=str(ser1.readline(),'utf-8')
          data=str_data.split(':')
          id=data[0]
          if id=='1':
             illumination=data[1]
          elif id=='2':
              smoke=data[1]
          elif id=='3':
              temperature=data[1]
              humidity=data[2]
#ser1=init_serial()
#str1 = str(ser1.readline())
temperature='0'  #温度
humidity='0'     #湿度
illumination ='0'  #光照
smoke='0' #烟雾
ser1 = serial.Serial('com8', 115200)
_thread.start_new_thread(Rec_serial,())
print(40*' '+'欢迎进入智能家居灯光系统！')
'''choice_device=input("选择设备: ")
choice_device=int(choice_device)
print(choice_device)'''

while True:
    print(50*' '+'1.客厅')
    print(50*' '+'2.餐厅')
    print(50*' '+'3.卧室')
    print(50*' '+'4.关闭所有灯光')
    print(50*' '+'5.环境数据')
    choice_device = input(50*' '+"选择设备: ")
    choice_device = int(choice_device)
    print('\n')
    if choice_device == 1:
        while True:
            print(50*' '+'11.普通模式')
            print(50*' '+'12.迎宾模式')
            print(50*' '+'13.影院模式')
            print(50*' '+'14.聚会模式')
            print(50*' '+'15.关闭灯光')
            print(50*' '+'16.返回')
            choice_mode = input(50*' '+"选择模式: ")
            if choice_mode=='16':
               break
            Parlor_Choice_mode(choice_mode)
    elif choice_device == 2:
        while True:
            print(50*' '+'21.正常模式')
            print(50*' '+'22.宾客模式')
            print(50*' '+'23.关闭灯光')
            print(50*' '+'24.返回')
            choice_mode = input(50*' '+"选择模式: ")
            if choice_mode=='24':
               break
            Kitchen_Choice_mode(choice_mode)
    elif choice_device == 3:
        while True:
            print(50*' '+'31.正常模式')
            print(50*' '+'32.睡觉模式')
            print(50*' '+'33.起床模式')
            print(50*' '+'34.关闭灯光')
            print(50*' '+'35.返回')
            choice_mode = input(50*' '+"选择模式: ")
            if choice_mode=='35':
               break
            Bedroom_Choice_mode(choice_mode)
    elif choice_device ==4:
        Close_ALL()
    elif choice_device == 5:
         print(50*' '+('客厅光照：%s' % illumination))
         print(50*' '+('厨房烟雾：%s' % smoke))
         print(50*' '+('卧室温度：%s' % temperature))
         print(50*' '+('卧室湿度：%s' % humidity))

    print('\n')
print('hello')