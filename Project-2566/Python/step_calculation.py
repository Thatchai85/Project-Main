X = float(input("insert X value(Pixel) : "))
Y = float(input("insert Y value(Pixel) : "))

def convertPX2Step(X, Y):
    m2s = 0.2 # mm/step100
    Xfactor = 1.18518519 #px/mm.
    Yfactor = 1.17073 #px/mm.
    Xspace = 160
    Yspace = 0
    X = (((640 - X)/Xfactor)/m2s) + Xspace
    Y = (((480 - Y)/Yfactor)/m2s) - Yspace
    return X , Y

new_X, new_Y = convertPX2Step(X, Y)
print(f"from ({X},{Y}) converted to be ({round(new_X,0)},{round(new_Y,0)})")
#print("X: ",int(new_X) , "Y: ",int(new_Y))