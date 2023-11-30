import cv2 as cv
import cv2.aruco as aruco

# Charger une image depuis la caméra (0 pour la caméra par défaut, généralement la webcam)
cap = cv.VideoCapture(0)


# Définir le dictionnaire ArUco
aruco_dict = aruco.getPredefinedDictionary(aruco.DICT_6X6_250)

# Créer le détecteur ArUco
parameters = aruco.DetectorParameters()

#
detector = aruco.ArucoDetector(aruco_dict,parameters)

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

    # Afficher l'image
    cv.imshow('Frame', frame)

    # Sortir de la boucle si la touche 'q' est enfoncée
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

# Libérer la capture et fermer la fenêtre
cap.release()
cv.destroyAllWindows()