import cv2 as cv
import cv2.aruco as aruco
import numpy as np

# Focal de la cam
# Calculer la focal avec le fichier get_the_focal.py
# FOCAL_LENGTH = 1444 # Téléphone Maxime pixel 7 pro
FOCAL_LENGTH = 1470
# FOCAL_LENGTH = 600

arucoTagMapping = {
    47: ["Panneau solaire", 3.62],
    36: ["Fleur blanche", 2],
    13: ["FLeur violette", 2]
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
        # aruco.drawDetectedMarkers(frame, corners, ids)

        for i in range(len(ids)):

            if len(corners) > 0:
                ids = ids.flatten()
                for (markerCorner, markerID) in zip(corners, ids):

                    if markerID not in arucoTagMapping:
                        continue

                    try:
                        tagCorners = markerCorner.reshape((4, 2))
                    except:
                        print("Error with the corners")
                        continue

                    (topLeft, topRight, bottomRight, bottomLeft) = tagCorners

                    topRight = (int(topRight[0]), int(topRight[1]))
                    topLeft = (int(topLeft[0]), int(topLeft[1]))
                    bottomRight = (int(bottomRight[0]), int(bottomRight[1]))
                    bottomLeft = (int(bottomLeft[0]), int(bottomLeft[1]))

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

                    # Calculate the difference in the x and y coordinates
                    dx = topRight[0] - topLeft[0]
                    dy = topRight[1] - topLeft[1]

                    # Calculate the angle in radians
                    angle_radians = np.arctan2(dy, dx)

                    # Convert the angle to degrees
                    angle_degrees = np.degrees(angle_radians)

                    # Print the rotation angle of the ArUco tag
                    sens_du_tag = ""
                    if -15 < angle_degrees < 15:
                        sens_du_tag = "debout"
                    elif 15 < angle_degrees < 60:
                        sens_du_tag = "penche a droite"
                    elif 60 < angle_degrees < 110:
                        sens_du_tag = "tombe a droite"
                    elif 110 < angle_degrees < 180:
                        sens_du_tag = "a l'envers"
                    elif -180 < angle_degrees < -110:
                        sens_du_tag = "a l'envers"
                    elif -110 < angle_degrees < -60:
                        sens_du_tag = "tombe a gauche"
                    elif -60 < angle_degrees < -15:
                        sens_du_tag = "penche a gauche"

                    # Affichage des informations
                    cv.putText(frame, f"{arucoTagMapping[markerID][0]}, id : {markerID}", (topLeft[0], topLeft[1] - 90),
                               cv.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)
                    distance_str = "{:.2f}".format(D)
                    cv.putText(frame, f"Distance : {distance_str} cm", (topLeft[0], topLeft[1] - 60),
                               cv.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)
                    angle_str = "{:.2f}".format(angle_to_tag_horizontal_deg)
                    cv.putText(frame, f"Angle horizontale : {angle_str} degrees", (topLeft[0], topLeft[1] - 30),
                               cv.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)
                    cv.putText(frame, f"Le tag est {sens_du_tag}", (topLeft[0], topLeft[1]),
                               cv.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)

                    # print(f"{arucoTagMapping[markerID][0]} : {angle_to_tag_horizontal_deg} degrees, {D} cm")
    # Afficher l'image
    cv.imshow('Frame', frame)

    # Sortir de la boucle si la touche 'q' est enfoncée
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

# Libérer la capture et fermer la fenêtre
cap.release()
cv.destroyAllWindows()
