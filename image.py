import cv2
import sys

print("Modelec")
print("Your OpenCV version is: " + cv2.__version__)

img = cv2.imread(cv2.samples.findFile("WIN_20231123_18_41_25_Pro.jpg"))

if img is None:
    sys.exit("Could not read the image.")

cv2.imshow("Display window", img)
k = cv2.waitKey(0)

if k == ord("s"):
    cv2.imwrite("WIN_20231123_18_41_25_Pro.jpg", img)