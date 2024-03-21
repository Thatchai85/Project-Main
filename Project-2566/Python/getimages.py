import cv2 as cv

cap = cv.VideoCapture(0)
num = 1

while cap.isOpened():
    succes, img = cap.read()
    k = cv.waitKey(5)
    if k == 27:
        break
    elif k == ord('s'):
        cv.imwrite('pic9x12/pictest' + str(num) + '.png', img)
        print("image saved!")
        num +=1
        
    cv.imshow("Img", img)
    
cap.release()
cv.destroyAllWindows
