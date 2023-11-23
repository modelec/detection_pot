import cv2 as cv
import numpy as np
import tensorflow as tf

model = tf.saved_model.load('Pot-plante.tfrecord')

cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Cannot open camera")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Can't receive frame (stream end?). Exiting ...")
        break

    # Prétraiter l'image pour l'entrée du modèle
    input_image = cv2.resize(frame, (300, 300))
    input_image = input_image / 127.5 - 1.0  # Normalisation

    # Effectuer la détection d'objets avec le modèle TensorFlow
    detections = model(tf.convert_to_tensor(np.expand_dims(input_image, axis=0), dtype=tf.float32))

    # Dessiner les boîtes englobantes sur l'image
    for detection in detections['detection_boxes'][0].numpy():
        ymin, xmin, ymax, xmax = detection
        xmin = int(xmin * frame.shape[1])
        xmax = int(xmax * frame.shape[1])
        ymin = int(ymin * frame.shape[0])
        ymax = int(ymax * frame.shape[0])

        cv2.rectangle(frame, (xmin, ymin), (xmax, ymax), (0, 255, 0), 2)

    # Afficher le cadre résultant
    cv2.imshow("Object Detection", frame)

    if cv2.waitKey(1) == ord("q"):
        break

cap.release()
cv2.destroyAllWindows()