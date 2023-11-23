import tensorflow as tf

# Définir la fonction pour parser les exemples TFRecord (similaire à ce que vous avez utilisé)
def _parse_function(proto):
    keys_to_features = {
        'image/encoded': tf.io.FixedLenFeature([], tf.string),
        'image/format': tf.io.FixedLenFeature([], tf.string),
        'image/object/class/label': tf.io.FixedLenFeature([], tf.int64),  # Assurez-vous que le type correspond
        'image/object/bbox/xmin': tf.io.FixedLenFeature([], tf.float32),
        'image/object/bbox/ymin': tf.io.FixedLenFeature([], tf.float32),
        'image/object/bbox/xmax': tf.io.FixedLenFeature([], tf.float32),
        'image/object/bbox/ymax': tf.io.FixedLenFeature([], tf.float32),
    }
    parsed_features = tf.io.parse_single_example(proto, keys_to_features)
    return parsed_features


# Charger les TFRecords
tfrecords_path = "Pot-plante.tfrecord"
dataset = tf.data.TFRecordDataset(tfrecords_path)

# Afficher les informations pour chaque exemple TFRecord
for raw_record in dataset.take(5):  # Prenez les 5 premiers exemples pour illustration
    parsed_record = _parse_function(raw_record.numpy())
    print(parsed_record)
