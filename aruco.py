import cv2 as cv
import cv2.aruco as aruco
import numpy as np

# Focal de la cam
# Calculer la focal avec le fichier get_the_focal.py
FOCAL_LENGTH = 600

arucoTagMapping = {
    47: ["Solar Panel", 2.2],
    36: ["White flower", 2.5],
    13: ["Purple Flower", 1.8]
}

# Charger une image depuis la caméra (0 pour la caméra par défaut, généralement la webcam)
cap = cv.VideoCapture(0)

# Définir le dictionnaire ArUco
aruco_dict = aruco.getPredefinedDictionary(aruco.DICT_4X4_50)

# Créer le détecteur ArUco
parameters = aruco.DetectorParameters()

#
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
        #aruco.drawDetectedMarkers(frame, corners, ids)

        for i in range(len(ids)):

            if len(corners) > 0:
                ids = ids.flatten()
                for (markerCorner, markerID) in zip(corners, ids):

                    if markerID not in arucoTagMapping:
                        continue

                    corners = markerCorner.reshape((4, 2))
                    (topLeft, topRight, bottomRight, bottomLeft) = corners

                    topRight = (int(topRight[0]), int(topRight[1]))
                    bottomRight = (int(bottomRight[0]), int(bottomRight[1]))
                    bottomLeft = (int(bottomLeft[0]), int(bottomLeft[1]))
                    topLeft = (int(topLeft[0]), int(topLeft[1]))

                    center_x = int((topLeft[0] + topRight[0] + bottomRight[0] + bottomLeft[0]) / 4)
                    center_y = int((topLeft[1] + topRight[1] + bottomRight[1] + bottomLeft[1]) / 4)

                    # Calculate the apparent width in pixels
                    P = np.sqrt((topRight[0] - topLeft[0]) ** 2 + (topRight[1] - topLeft[1]) ** 2)

                    # Calculate the distance to the Aruco tag
                    D = (arucoTagMapping[markerID][1] * FOCAL_LENGTH) / P

                    tag_position_x = (topRight[0] + topLeft[0]) / 2
                    tag_position_y = (topRight[1] + bottomRight[1]) / 2

                    image_width = frame.shape[1]
                    image_height = frame.shape[0]

                    # Calcul de l'angle horizontal par rapport au centre de l'image
                    angle_to_tag_horizontal = np.arctan2(tag_position_x - (image_width / 2),
                                                         image_width / (2 * np.tan(np.radians(60))))

                    # Conversion de l'angle en degrés
                    angle_to_tag_horizontal_deg = np.degrees(angle_to_tag_horizontal)

                    # Affichage des informations
                    cv.putText(frame, f"{arucoTagMapping[markerID][0]}", (topLeft[0], topLeft[1] - 45), cv.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
                    distance_str = "{:.2f}".format(D)
                    cv.putText(frame, f"Distance : {distance_str} cm", (topLeft[0], topLeft[1] - 30), cv.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
                    angle_str = "{:.2f}".format(angle_to_tag_horizontal_deg)
                    cv.putText(frame, f"Angle : {angle_str} degrees", (topLeft[0], topLeft[1] - 15), cv.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

                    print(f"{arucoTagMapping[markerID][0]} : {angle_to_tag_horizontal_deg} degrees, {D} cm")
    # Afficher l'image
    cv.imshow('Frame', frame)

    # Sortir de la boucle si la touche 'q' est enfoncée
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

# Libérer la capture et fermer la fenêtre
cap.release()
cv.destroyAllWindows()
