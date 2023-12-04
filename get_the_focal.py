import cv2 as cv
import cv2.aruco as aruco
import numpy as np

# Actual width of the Aruco tag in some unit (like centimeters)
W = 1.8

# Actual distance from the camera to the object in the same unit as W
D = 15.0

# Charger une image depuis la caméra (0 pour la caméra par défaut, généralement la webcam)
cap = cv.VideoCapture(0)

# Définir le dictionnaire ArUco
aruco_dict = aruco.getPredefinedDictionary(aruco.DICT_4X4_50)

# Créer le détecteur ArUco
parameters = aruco.DetectorParameters()

detector = aruco.ArucoDetector(aruco_dict, parameters)

while True:
    # Lire une image depuis la caméra
    ret, frame = cap.read()

    # Convertir l'image en niveaux de gris
    gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)

    # Détecter les marqueurs ArUco
    corners, ids, rejectedImgPoints = detector.detectMarkers(gray)

    # Dessiner les résultats
    if ids is not None:
        aruco.drawDetectedMarkers(frame, corners, ids)

        for i in range(len(ids)):

            # verify *at least* one ArUco marker was detected
            if len(corners) > 0:
                # Flatten the ArUco IDs list
                ids = ids.flatten()

                # Loop over the detected ArUCo corners
                for (markerCorners, markerID) in zip(corners, ids):
                    # Extract the marker corners
                    corners = markerCorners.reshape((4, 2))
                    (topLeft, topRight, bottomRight, bottomLeft) = corners

                    # Convert each of the (x, y)-coordinate pairs to integers
                    topRight = (int(topRight[0]), int(topRight[1]))
                    topLeft = (int(topLeft[0]), int(topLeft[1]))

                    # Calculate the apparent width in pixels
                    P = np.sqrt((topRight[0] - topLeft[0]) ** 2 + (topRight[1] - topLeft[1]) ** 2)

                    # Calculate the focal length
                    F = (P * D) / W
                    print(f"The focal length of the camera is: {F} pixels")

    # Afficher l'image
    cv.imshow('Frame', frame)

    # Sortir de la boucle si la touche 'q' est enfoncée
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

# Libérer la capture et fermer la fenêtre
cap.release()
cv.destroyAllWindows()
