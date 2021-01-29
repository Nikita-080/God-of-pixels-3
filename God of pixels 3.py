def Cloud():
    from CloudLib import CloudMap
    global img
    res=round(widget[6][6].get()*s/20)
    octaves=widget[6][7].get()
    unbias=final_correct.get()
    img3=CloudMap(s,res,octaves,unbias)
    k1=(48+2*(26-widget[6][8].get()))/255
    for i in range(s):
        for k in range(s):
            if img.getpixel((i,k))!=(0,0,0):
                koef=img3.getpixel((i,k))*k1
                color_res=[0,0,0]
                for j in range(3):
                    a=colors[7][j]
                    p=img.getpixel((i,k))[j]
                    if a>p:
                        color_res[j]=round((a-p)/100*koef+p)
                    elif a<p:
                        color_res[j]=round(p-(p-a)/100*koef)
                    else:
                        color_res[j]=a
                img.putpixel((i,k),tuple(color_res))
def Atmosphere():
    import math
    global R2, img
    R2=R*(81+widget[7][4].get())/81 #радиус атмосферы
    w=21-widget[7][2].get() # уровень прозрачности
    l_min=R2-R #минммальная толщина атмосферы
    l_max=2*math.sqrt(R2*R-R*R) #максимальная толщина
    k1=(5*w-100)/(l_min-l_max) #коэффициенты функции изменения прозрачности
    k2=5*w-k1*l_min
    for i in range(s):
        for k in range(s):
            if (i-s//2)*(i-s//2)+(k-s//2)*(k-s//2)<=R2*R2:
                img.putpixel((i,k),Atmosphere_step(i,k,img.getpixel((i,k)),k1,k2))
def Atmosphere_step(x,y,color_planet,k1,k2):
    import math
    global R2
    x=x-s//2 #преобразование комп. системы координат
    y=s//2-y #в математическую
    r=math.sqrt(x*x+y*y) #расстояние до центра
    if R<=r<=R2:
        l=2*math.sqrt(R2*r-r*r) #приведенная толщина атмосферы
    else:
        l=math.sqrt(r*R2-r*r)-math.sqrt(r*R-r*r)
    k=l*k1+k2 #коэффициент прозрачности
    color_res=[0,0,0]
    for i in range(3):
        a=colors[8][i]
        p=color_planet[i]
        if a>p:
            color_res[i]=round((a-p)/100*k+p)
        elif a<p:
            color_res[i]=round(p-(p-a)/100*k)
        else:
            color_res[i]=a
    return tuple(color_res)
def ColorChange(adress):
    global colors
    from tkinter import colorchooser
    from math import floor
    new_color=colorchooser.askcolor(color=colors[adress])[0]
    if new_color!=None:
        new_color=(floor(new_color[0]),floor(new_color[1]),floor(new_color[2]))
        colors[adress]=new_color
        if adress==7:
            widget[6][5].config(bg="#"+('%02x%02x%02x' % new_color))
        elif adress==8:
            widget[7][5].config(bg="#"+('%02x%02x%02x' % new_color))
        else:
            widget[2][adress+9].config(bg="#"+('%02x%02x%02x' % new_color))
def UV_step(x,y):
    import math
    x=x-s//2 #преобразование комп. системы координат
    y=s//2-y #в математическую
    r=math.sqrt(x*x+y*y) #расстояние до центра
    l=math.sin(r/R)*R #проекционная длина
    xn=x*l/r #новые координаты
    yn=y*l/r
    xn=round(xn+s//2) #в комп. систему
    yn=round(s//2-yn)
    return xn,yn
def UV():
    from PIL import Image
    global img
    img2=img.copy()
    for i in range(s):
        for k in range(s):
            img.putpixel((i,k),(0,0,0))
    count=0
    for i in range(s):
        for k in range(s):
            if i==k==s//2:
                img.putpixel((i,k),img2.getpixel((i,k)))
            else:
                if img2.getpixel((i,k))!=(0,0,0):
                    img.putpixel(UV_step(i,k),img2.getpixel((i,k)))
def Center(coords):
    return [(coords[0]+coords[2])//2,(coords[1]+coords[3])//2]
def F21(event):
    global ball
    a=widget[4][3].find_overlapping(event.x,event.y,event.x,event.y)
    if ball[0] in a:
        ball[1]=True
def F22(event):
    global x2,y2,ball,line1,line2
    coords=widget[4][3].bbox(ball[0])
    if ball[1]==True and circle not in widget[4][3].find_overlapping(coords[0]+event.x-x2,coords[1]+event.y-y2,coords[2]+event.x-x2,coords[3]+event.y-y2):
        widget[4][3].move(ball[0],event.x-x2,event.y-y2)
        widget[4][3].move(line1,event.x-x2,0)
        widget[4][3].move(line2,0,event.y-y2)
    x2=event.x
    y2=event.y
def F23(event):
    global ball
    ball[1]=False
def F1(event): #захват
    global objects
    a=widget[1][1].find_overlapping(event.x,event.y,event.x,event.y)
    for i in range(1,7):
        if objects[i][0] in a:
            objects[i][1]=True
def F2(event): #движение
    global y
    for i in range(8):
        if objects[i][1]==True:
            y_up=widget[1][1].bbox(objects[i-1][0])[1]
            y_mid=widget[1][1].bbox(objects[i][0])[1]+event.y-y
            y_down=widget[1][1].bbox(objects[i+1][0])[1]
            if y_up<=y_mid<=y_down:
                widget[1][1].move(objects[i][0],0,event.y-y)
    y=event.y
    for i in range(7):
        y_up=widget[1][1].bbox(objects[i][0])[1]
        y_mid=widget[1][1].bbox(Text[i])[1]
        y_down=widget[1][1].bbox(objects[i+1][0])[1]
        widget[1][1].move(Text[i],0,(y_up+y_down)//2-y_mid-4)
def F3(event): #отпускание
    global objects
    for i in objects:
        i[1]=False
def Info():
    from tkinter import messagebox as mb
    txt.insert(INSERT, "Showing information...\n")
    mb.showinfo("Справка","Название - God of pixels 3\nВерсия - 3/0.0.1\nРазработчик - Рябов Никита\nОбратная связь - riabovnick080@yandex.ru")
    txt.insert(INSERT, "    result - successfully\n")
def Save():
    from tkinter import messagebox as mb
    from tkinter import filedialog
    global img, name
    txt.insert(INSERT, "Saving...\n")
    if img==0:
        mb.showerror("Ошибка","Изображение не создано")
        txt.insert(INSERT, "    result - failed\n")
    else:
        path=filedialog.asksaveasfilename(initialfile="Planet "+name,defaultextension='.png')
        if path=="":
            txt.insert(INSERT, "    result - cancelled\n")
        else:
            img.save(path)
            txt.insert(INSERT, "    result - successfully\n")
def Name():
    import random
    if name_format.get()==0 or name_format.get()==1:
        sogl=list("rtpsdfghkljzxcvbnmy")
        glas=list("euioa")
        s=""
        x=random.randint(3,8)
        s+=random.choice(sogl+glas)
        s+=random.choice(sogl+glas)
        for i in range(x-2):
            if s[-1] in sogl and s[-2] in sogl:
                s+=random.choice(glas)
            elif s[-1] in glas and s[-2] in glas:
                s+=random.choice(sogl)
            else:
                s+=random.choice(sogl+glas)
        if name_format.get()==0:
            return s+"-"+str(random.randint(1,1000))
        else:
            return s[0].upper()+s[1:]
    elif name_format.get()==2:
        s=""
        alfa="RSTUVWXYZ"
        alfa2="qwertyuiopsdfghjklzxcvbnm"
        star_systems=["And","Gem","UMa","CMa","Lib","Aqr","Aur","Lup","Boo","Com",
                      "Crv","Her","Hya","Col","CVn","Vir","Del","Dra","Mon","Ara",
                      "Ara","Pic","Cam","Gru","Oph","Ser","Dor","Ind","Car","Cet",
                      "Cap","Pyx","Pup","Cyg","Leo","Leo","Lyr","Vul","UMi","Equ",
                      "LMi","CMi","Mic","Mus","Ant","Nor","Ari","Oct","Aql","Ori",
                      "Pav","Vel","Peg","Per","For","Aps","Cnc","Cae","Psc","Lyn",
                      "Ret","Scu","Men","Sge","Sgr","Tel","Tau","Tri","Tuc","Phe",
                      "Cha","Cen","Cep","Cir","Hor","Crt","Sct","Eri","Hyi","CrA",
                      "PsA","Cru","TrA"]
        s+=random.choice(alfa)
        if random.randint(0,1)==0:
            s+=random.choice(alfa)
        return s+" "+random.choice(star_systems)+" "+random.choice(alfa2)
    elif name_format.get()==3:
        alfa="qwertyuiopasdfghjklzxcvbnm1234567890"
        s=""
        for i in range(random.randint(3,10)):
            s+=random.choice(alfa)
        return s
    elif name_format.get()==4:
        alfa="0123456789"
        s="id"
        for i in range(9):
            s+=random.choice(alfa)
        return s
    elif name_format.get()==5:
        s=""
        alfa="qwertyuiopasdfghjklzxcvbnm1234567890QWERTYUIOPASDFGHJKLZXCVBNM"
        for i in range(4):
            for k in range(4):
                s+=random.choice(alfa)
            s+="-"
        return s[:-1]
def Plant():
    import random, math
    global img
    for i in range(s):
        for k in range(s):
            if img.getpixel((i,k)) in (colors[2],colors[3]):
                r_e=abs((s//2-k)/(s//2))
                if life_w[2].get()*100+random.randint(0,round((1-r_e)*1000))>1000:
                    img.putpixel((i,k),plant)
def Human():
    import random,math
    global img
    city=[]
    v=round(math.pi*R*R/3*(life_w[4].get()*-0.1+1.5)) #вероятность появления городской аггломерации
    for i in range(s):
        for k in range(s):
            if img.getpixel((i,k)) in (colors[2],colors[3],plant):
                if random.randint(0,v)==1:
                    city.append([i,k])
                    img.putpixel((i,k),human)
                    img.putpixel((i+1,k),human)
                    img.putpixel((i-1,k),human)
                    img.putpixel((i,k+1),human)
                    img.putpixel((i,k-1),human)
    if len(city)==0:
        return
    for i in range(s):
        for k in range(s):
            if img.getpixel((i,k)) in (colors[2],colors[3],plant):
                r=min([math.sqrt((x[0]-i)*(x[0]-i)+(x[1]-k)*(x[1]-k)) for x in city]) #расстояние до ближайшей аггломерации
                p=round(G(r)*(life_w[4].get()/10+0.5)) #вероятность в промилле
                if random.randint(0,1000)<p:
                    img.putpixel((i,k),human)
def G(r):
    import math
    k2=(500-100*R)/-450
    k1=100*R+50*k2
    return k1/(2*r+k2)
def Shadow(x1,y1): #координаты пикселя
    import math
    global R2
    if is_atmo.get()==True:
        Rs=R2
    else:
        Rs=R
    a=x1-s//2 #координаты относительно центра картинки
    b=s//2-y1
    xm=(Center(widget[4][3].bbox(ball[0]))[0]-115)*Rs/115  #координаты точки максимальной освещенности (ТМО)
    ym=(115-Center(widget[4][3].bbox(ball[0]))[1])*Rs/115
    zm=math.sqrt(Rs*Rs-xm*xm-ym*ym)
    if Rs*Rs-a*a-b*b<0:
        return 0
    z=math.sqrt(Rs*Rs-a*a-b*b) #координата точки на поверхности планеты
    r=math.sqrt((a-xm)*(a-xm)+(b-ym)*(b-ym)+(z-zm)*(z-zm)) #расстояние до ТМО
    u=math.degrees(math.asin(r/2/Rs)) #угловое расстояние до ТМО
    k=(45-u)/45 #теневой коэффициент
    return k
def God(tip): #генерация планеты
    import random, math
    from PIL import ImageTk, Image
    from datetime import datetime
    from diamond_square_i import Matrix
    global img, name, m,h,s,d,R
    if m==0 and tip==2:
        txt.insert(INSERT, "Planet modification...\n")
        txt.insert(INSERT, "    result - failed\n")
        return
    if tip==1:
        txt.insert(INSERT, "Planet generation...\n")
    else:
        txt.insert(INSERT, "Planet modification...\n")
    g_t=datetime.now().second #время всей генерации
    pb['value']=0
    window.update_idletasks()
    #создание карты высот
    if tip==1:
        m,h,s,d=Matrix(common_w[2].get(),common_w[6].get(),common_w[4].get())
    R=s//2*2/math.pi
    pb['value']=10
    window.update_idletasks()
    #коррекция высот
    if tip==1:
        if d>0:
            d=0
        for i in range(s):
            for k in range(s):
                m[i][k]=(m[i][k]-d)*280/(h-d)
    pb['value']=20
    window.update_idletasks()
    #создание уровней
    level=[]
    for i in range(7):
        up=Center(widget[1][1].bbox(objects[i][0]))[1]
        down=Center(widget[1][1].bbox(objects[i+1][0]))[1]
        up=280-(up-5)
        down=280-(down-5)
        if up!=down:
            level.append([up,down,colors[i]])
    pb['value']=30
    window.update_idletasks()
    #создание картинки
    img=Image.new('RGB',(s,s))
    for i in range(s):
        for k in range(s):
            if (i-s//2-1)*(i-s//2-1)+(k-s//2-1)*(k-s//2-1)<=(s//2)*(s//2):
                for j in level:
                    if j[0]>=m[i][k]>=j[1]:
                        img.putpixel((i,k),j[2])
                        break
    pb['value']=40
    window.update_idletasks()
    #создание растений
    if life_w[2].get()>0:
        Plant()
    pb['value']=50
    window.update_idletasks()
    #создание людей
    if life_w[4].get()>0:
        Human()
    pb['value']=60
    window.update_idletasks()
    #облака
    if is_cloud.get()==True:
        Cloud()
    #UV
    UV()
    pb['value']=70
    window.update_idletasks()
    #атмосфера
    if is_atmo.get()==True:
        Atmosphere()
    #тени
    for i in range(s):
        for j in range(s):
            a=img.getpixel((i,j))[0]
            b=img.getpixel((i,j))[1]
            c=img.getpixel((i,j))[2]
            if (a,b,c)!=(0,0,0) and (a,b,c)!=human:
                k=Shadow(i,j)
                if k<0:
                    img.putpixel((i,j),(0,0,0))
                else:
                    k*=widget[4][1].get()*0.1+0.5
                    a=round(a*k)
                    b=round(b*k)
                    c=round(c*k)
                    img.putpixel((i,j),(a,b,c))
    pb['value']=80
    window.update_idletasks()
    #вывод на экран
    if tip==1:
        name=Name()
    name_lbl.config(text=name)
    if s<300:
        img=img.resize((257, 257), Image.ANTIALIAS)
        img2=ImageTk.PhotoImage(img)
    else:
        img2=img.resize((257, 257), Image.ANTIALIAS)
        img2=ImageTk.PhotoImage(img2)
    btn_pic.config(image=img2)
    btn_pic.image=img2
    pb['value']=100
    window.update_idletasks()
    txt.insert(INSERT, "    result - successfully\n    time - "+str(datetime.now().second-g_t)+"s\n")
def settings(adress): #перемещение между вкладками
    x=adress.grid_info()["row"]
    for i in range(8):
        btn_m[i].config(bg="#f0f0ed")
    btn_m[x].config(bg="#a8a8c2")
    for i in widget:
        for k in i:
            k.grid_remove()
    for i in widget[x]:
        i.grid()
from tkinter import *
import tkinter.ttk as ttk
window=Tk()
window.title("God of pixels")
#========== создание вкладок
a=["Общие","Структура","Цвет","Жизнь","Свет","Имя","Облака","Атмосфера"]
btn_m=[]
for i in range(8):
    btn_m.append(Button(text=a[i],font="Arial 18"))
    btn_m[i].config(command=lambda adress=btn_m[i]: settings(adress))
    btn_m[i].grid(row=i,column=0,sticky=W+E)
#========== задание переменных
name_format=IntVar()
final_correct=BooleanVar()
is_cloud=BooleanVar()
is_atmo=BooleanVar()
#========== цвета
colors=[(255,255,255), #0 - ледники
        (70,70,70),    #1 - скалы
        (128,128,128), #2 - горы
        (180,180,180), #3 - равнины
        (252,221,118), #4 - пляж
        (40,40,255),   #5 - шельф
        (0,0,128),     #6 - океан
        (217,217,217), #7 - облака
        (183,237,253)] #8 - атмосфера
plant=(93,161,48)
human=(255,255,0)
#========== виджеты
common_w=[Label(text="Общие",font="Arial 12"),
          Label(text="Размер мира",font="Arial 12"),
          Scale(orient=HORIZONTAL,length=250,from_=4,to=10,resolution=1),
          Label(text="Начальная высота",font="Arial 12"),
          Scale(orient=HORIZONTAL,length=250,from_=1,to=256,resolution=2),
          Label(text="Случайность",font="Arial 12"),
          Scale(orient=HORIZONTAL,length=250,from_=1,to=50,resolution=1)]
struk_w=[Label(text="Структура",font="Arial 12"),
         Canvas(width=230,height=288,bg='white')]
color_w=[Label(text="Биом",font="Arial 12"),
         Label(text="ледники",font="Arial 10"),
         Label(text="скалы",font="Arial 10"),
         Label(text="горы",font="Arial 10"),
         Label(text="равнины",font="Arial 10"),
         Label(text="пляжи",font="Arial 10"),
         Label(text="шельф",font="Arial 10"),
         Label(text="океан",font="Arial 10"),
         Label(text="Цвет",font="Arial 12"),
         Button(bg="#"+('%02x%02x%02x' % colors[0]),command=lambda adress=0: ColorChange(adress),width=15),
         Button(bg="#"+('%02x%02x%02x' % colors[1]),command=lambda adress=1: ColorChange(adress),width=15),
         Button(bg="#"+('%02x%02x%02x' % colors[2]),command=lambda adress=2: ColorChange(adress),width=15),
         Button(bg="#"+('%02x%02x%02x' % colors[3]),command=lambda adress=3: ColorChange(adress),width=15),
         Button(bg="#"+('%02x%02x%02x' % colors[4]),command=lambda adress=4: ColorChange(adress),width=15),
         Button(bg="#"+('%02x%02x%02x' % colors[5]),command=lambda adress=5: ColorChange(adress),width=15),
         Button(bg="#"+('%02x%02x%02x' % colors[6]),command=lambda adress=6: ColorChange(adress),width=15)]
life_w=[Label(text="Жизнь",font="Arial 12"),
        Label(text="Количество растений",font="Arial 12"),
        Scale(orient=HORIZONTAL,length=250,from_=0,to=10,resolution=1),
        Label(text="Количество людей",font="Arial 12"),
        Scale(orient=HORIZONTAL,length=250,from_=0,to=10,resolution=1)]
shine_w=[Label(text="Уровень освещения",font="Arial 12"),
         Scale(orient=HORIZONTAL,length=250,from_=0,to=10,resolution=1),
         Label(text="Точка падения света",font="Arial 12"),
         Canvas(width=231,height=231,bg='white')]
name_w=[Label(text="Имя планеты",font="Arial 12"),
        Radiobutton(text="стандарт GoP2",font="Arial 12",variable=name_format,value=0),
        Radiobutton(text="человекочитаемое имя",font="Arial 12",variable=name_format,value=1),
        Radiobutton(text="астрономический стандарт",font="Arial 12",variable=name_format,value=2),
        Radiobutton(text="абсолютно случайное",font="Arial 12",variable=name_format,value=3),
        Radiobutton(text="id",font="Arial 12",variable=name_format,value=4),
        Radiobutton(text="ключ продукта",font="Arial 12",variable=name_format,value=5)]
cloud_w=[Checkbutton(text="Облака",variable=is_cloud,font="Arial 12"),
         Label(text="размер облаков",font="Arial 12"),
         Label(text="детализация",font="Arial 12"),
         Label(text="прозрачность",font="Arial 12"),
         Checkbutton(text="коррекция",variable=final_correct,font="Arial 12"),
         Button(text="Цвет",font="Arial 12",bg="#"+('%02x%02x%02x' % colors[7]),command=lambda adress=7: ColorChange(adress),width=15),
         Scale(orient=HORIZONTAL,length=120,from_=1,to=20,resolution=1),
         Scale(orient=HORIZONTAL,length=120,from_=1,to=10,resolution=1),
         Scale(orient=HORIZONTAL,length=120,from_=1,to=25,resolution=1)]
atmo_w=[Checkbutton(text="Атмосфера",variable=is_atmo,font="Arial 12"),
        Label(text="прозрачность",font="Arial 12"),
        Scale(orient=HORIZONTAL,length=250,from_=1,to=20,resolution=1),
        Label(text="размер",font="Arial 12"),
        Scale(orient=HORIZONTAL,length=250,from_=1,to=10,resolution=1),
        Button(text="Цвет",font="Arial 12",bg="#"+('%02x%02x%02x' % colors[8]),command=lambda adress=8: ColorChange(adress),width=15)]
widget=[common_w,struk_w,color_w,life_w,shine_w,name_w,cloud_w,atmo_w]
#========== настройка умолчаний
common_w[2].set(8)
common_w[4].set(128)
common_w[6].set(8)
shine_w[1].set(5)
final_correct.set(True)
cloud_w[6].set(2)
cloud_w[7].set(3)
cloud_w[8].set(1)
atmo_w[2].set(10)
atmo_w[4].set(7)
#========== настройка Canvas
widget[1][1].create_line(10, 4, 10, 284, fill="black")
objects=[]
for i in range(8):
    objects.append([widget[1][1].create_oval(6,i*40,14,i*40+8,fill="red"),False])
bioms=["ледники","скалы","горы","равнины","пляжи","шельф","океан"]
Text=[]
for i in range(7):
    Text.append(widget[1][1].create_text(50,21+i*40,text=bioms[i]))
y=0
widget[1][1].bind("<ButtonPress-1>", F1)
widget[1][1].bind("<Motion>", F2)
widget[1][1].bind("<ButtonRelease-1>", F3)
#========== настройка Canvas 2
circle=widget[4][3].create_oval(2,2,228,228)
widget[4][3].create_line(115, 0, 115, 230, fill="black")
widget[4][3].create_line(0, 115, 230, 115, fill="black")
line1=widget[4][3].create_line(89, 0, 89, 230, fill="red",dash=(2,2))
line2=widget[4][3].create_line(0, 115, 230, 115, fill="red",dash=(2,2))
ball=[widget[4][3].create_oval(85,111,93,119,fill="red"),False]
x2=0
y2=0
widget[4][3].bind("<ButtonPress-1>", F21)
widget[4][3].bind("<Motion>", F22)
widget[4][3].bind("<ButtonRelease-1>", F23)
#========== размещение виджетов
for i in range(8):
    for k in range(len(widget[i])):
        widget[i][k].grid(row=k,column=1,columnspan=2,sticky=W)
        widget[i][k].grid_remove()
#========== исключения (особое размещение)
widget[1][1].grid(row=1,column=1,rowspan=7,sticky=W)
widget[4][3].grid(row=3,column=1,rowspan=5,sticky=W)
widget[2][6].grid(row=6,column=1,sticky=W)
widget[2][7].grid(row=7,column=1,sticky=W)
widget[2][8].grid(row=6,column=2,sticky=W)
widget[2][9].grid(row=7,column=2,sticky=W)
widget[6][6].grid(row=1,column=2)
widget[6][7].grid(row=2,column=2)
widget[6][8].grid(row=3,column=2)
widget[6][6].grid_remove()
widget[6][7].grid_remove()
widget[6][8].grid_remove()
widget[1][1].grid_remove()
widget[4][3].grid_remove()
widget[2][6].grid_remove()
widget[2][7].grid_remove()
widget[2][8].grid_remove()
widget[2][9].grid_remove()
for i in range(8):
    widget[2][i].grid(row=i,column=1)
    widget[2][i+8].grid(row=i,column=2)
    widget[2][i].grid_remove()
    widget[2][i+8].grid_remove()
#========== показ первой вкладки
for i in widget[0]:
    i.grid()
btn_m[0].config(bg="#a8a8c2")
#========== еще виджеты
sep=ttk.Separator(orient="horizontal")
sep.grid(row=8,column=1,columnspan=2,ipadx=130)
name_lbl=Label(text="Планета",font="Arial 18")
name_lbl.grid(row=0,column=3,columnspan=2,sticky=E+W)
from PIL import ImageTk, Image
btn_pic=Button(window)
btn_pic.grid(row=1,column=3,rowspan=6,columnspan=2,sticky=W+E+S+N)
btn1=Button(text="Создать",font="Arial 18",command=lambda tip=1: God(tip))
btn1.grid(row=7,column=3,sticky=E+W)
btn2=Button(text="Изменить",font="Arial 18",command=lambda tip=2: God(tip))
btn2.grid(row=7,column=4,sticky=E+W)
pb=ttk.Progressbar(length=300,mode='determinate')
pb.grid(row=8,column=3,columnspan=2)
#строка состояния
from tkinter import scrolledtext
txt=scrolledtext.ScrolledText(width=25, height=20)  
txt.grid(column=5, row=0, rowspan=8, columnspan=3)
#меню
mainmenu = Menu()
filemenu = Menu(mainmenu, tearoff=0)
filemenu.add_command(label='Сохранить изображение',command=Save)
mainmenu.add_cascade(label='Файл',menu=filemenu)
mainmenu.add_command(label='Справка',command=Info)
window.config(menu=mainmenu)
#глобальные переменные
m=0;h=0;s=0;d=0;R=0;R2=0
img=0
name=0
#==========
window.mainloop()
