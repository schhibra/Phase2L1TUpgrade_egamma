##########################################
import setGPU
import os
###for CPU
#os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
import h5py
import glob
import numpy as np

import tensorflow as tf

import matplotlib.pyplot as plt

from tensorflow.compat.v1 import ConfigProto
from tensorflow.compat.v1 import InteractiveSession
config = ConfigProto()
#####config.gpu_options.per_process_gpu_memory_fraction = 0.8
config.gpu_options.allow_growth = True
session = InteractiveSession(config=config)
##########################################

##########################################                                                                                                                                                   
def get_file_list(path):
    flist = []
    flist += glob.glob(path + '/' + '*.h5')
    flist.sort()
    print("flist: ", flist)
    return flist

def read_images_from_file(fname):
    print("Appending %s" %fname)
    with h5py.File(fname,'r') as f:
        ImageTrk  = np.array(f.get("ImageTrk_PUcorr")[:], dtype=np.float16)
        ImageTrk  = ImageTrk.reshape(ImageTrk.shape[0], ImageTrk.shape[1], ImageTrk.shape[2], 1)

        ImageECAL = np.array(f.get("ImageECAL")[:], dtype=np.float16)
        ImageECAL = ImageTrk.reshape(ImageECAL.shape[0], ImageECAL.shape[1], ImageECAL.shape[2], 1)

        ImageHCAL = np.array(f.get("ImageHCAL")[:], dtype=np.float16)
        ImageHCAL = ImageTrk.reshape(ImageHCAL.shape[0], ImageHCAL.shape[1], ImageHCAL.shape[2], 1)
        
        Image3D = np.concatenate([ImageTrk, ImageECAL, ImageHCAL], axis=-1)

        Image3D_zero = np.zeros((Image3D.shape[0], 288, 360, 3), dtype=np.float16)
        Image3D_zero[:, 1:287, :, :] += Image3D
        Image3D_zero = np.divide(Image3D_zero, 2000., dtype=np.float16)
        return Image3D_zero
        
def concatenate_by_file_content(Image3D, fname):
    Image3D_tmp = read_images_from_file(fname)
    Image3D = np.concatenate([Image3D, Image3D_tmp], axis=0) if Image3D.size else Image3D_tmp
    return Image3D
    
def gen(parts_n, pathindex):
    
    if   (pathindex == 0): flist = get_file_list("/eos/cms/store/cmst3/user/schhibra/ML_02042022/jobs_qcd_v1/gensim/output/train")
    elif (pathindex == 1): flist = get_file_list("/eos/cms/store/cmst3/user/schhibra/ML_02042022/jobs_qcd_v1/gensim/output/val"  )

    Image3D_conc = np.array([])
    
    for i_file, fname in enumerate(flist):
        Image3D_conc = concatenate_by_file_content(Image3D_conc, fname)
        
        while (len(Image3D_conc) >= parts_n):
            Image3D_part, Image3D_conc = Image3D_conc[:parts_n] , Image3D_conc[parts_n:]
            #print (" ",Image3D_part.shape, Image3D_conc.shape)
            yield (Image3D_part, Image3D_part)


parts = 128
dataset_train = tf.data.Dataset.from_generator(
    gen, 
    args=[parts, 0],
    output_types=(tf.float16, tf.float16))

dataset_val = tf.data.Dataset.from_generator(
    gen, 
    args=[parts, 1],
    output_types=(tf.float16, tf.float16))
##########################################

##########################################
# keras imports
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Dense, Input, Conv2D, Conv2DTranspose, Dropout, Flatten, Reshape, Lambda, Layer, LeakyReLU
from tensorflow.keras.layers import AveragePooling2D, BatchNormalization, Activation
from tensorflow.keras import backend as K
from tensorflow.keras.callbacks import EarlyStopping, ReduceLROnPlateau, TerminateOnNaN, ModelCheckpoint
from tensorflow.keras.optimizers import Adam
##########################################

##########################################
img_rows = 288
img_cols = 360
img_chns = 3

#wrt v1_1, I changed Conv2D(, (2,2)) to Conv2D(, (3,3))
#and decoder strides exactly same as encoder

############ENCODER
inputImage = Input(shape=(img_rows, img_cols, img_chns))
x1 = Conv2D(32, (3,3), padding="same")(inputImage)
x2 = BatchNormalization()(x1)
x3 = Activation('elu')(x2)
x4 = AveragePooling2D((3,3), padding="same")(x3)
x5 = Conv2D(64, (3,3), padding="same")(x4)
x6 = BatchNormalization()(x5)
x7 = Activation('elu')(x6)
x8 = AveragePooling2D((2,2), padding="same")(x7)
x9 = Conv2D(64, (3,3), padding="same")(x8)
x10 = BatchNormalization()(x9)
x11 = Activation('elu')(x10)
x12 = AveragePooling2D((2,2), padding="same")(x11)
x13 = Conv2D(64, (3,3), padding="same")(x12)
x14 = BatchNormalization()(x13)
x15 = Activation('elu')(x14)
x16 = AveragePooling2D((2,2), padding="same")(x15)
x17 = Conv2D(64, (2,2), padding="same")(x16)
x18 = BatchNormalization()(x17)
x19 = Activation('elu')(x18)
x20 = AveragePooling2D((3,3), padding="same")(x19)

x21 = Conv2D(64, (2,2), padding="same")(x20)
x22 = BatchNormalization()(x21)
encoder_output = Activation('elu')(x22)

############DECODER
x23 = Conv2DTranspose(64, (2,2), strides=(2, 2), padding="same")(encoder_output)
x24 = BatchNormalization()(x23)
x25 = Activation('elu')(x24)
x26 = Conv2DTranspose(64, (2,2), strides=(2, 2), padding="same")(x25)
x27 = BatchNormalization()(x26)
x28 = Activation('elu')(x27)
x29 = Conv2DTranspose(64, (2,2), strides=(2, 2), padding="same")(x28)
x30 = BatchNormalization()(x29)
x31 = Activation('elu')(x30)
x32 = Conv2DTranspose(32, (3,3), strides=(3, 3), padding="same")(x31)
x33 = BatchNormalization()(x32)
x34 = Activation('elu')(x33)
x35 = Conv2DTranspose(img_chns, (3,3), strides=(3, 3), padding="same")(x34)
x36 = BatchNormalization()(x35)
output = Activation('relu')(x36)

model = Model(inputs=inputImage, outputs=output)
encoder = Model(inputs=inputImage, outputs=encoder_output)

model = Model(inputs=inputImage, outputs=output)
encoder = Model(inputs=inputImage, outputs=encoder_output)
m1 = Model(inputs=inputImage, outputs=x1)
m2 = Model(inputs=inputImage, outputs=x2)
m3 = Model(inputs=inputImage, outputs=x3)
m4 = Model(inputs=inputImage, outputs=x4)
m5 = Model(inputs=inputImage, outputs=x5)
m6 = Model(inputs=inputImage, outputs=x6)
m7 = Model(inputs=inputImage, outputs=x7)
m8 = Model(inputs=inputImage, outputs=x8)
m9 = Model(inputs=inputImage, outputs=x9)
m10 = Model(inputs=inputImage, outputs=x10)
m11 = Model(inputs=inputImage, outputs=x11)
m12 = Model(inputs=inputImage, outputs=x12)
m13 = Model(inputs=inputImage, outputs=x13)
m14 = Model(inputs=inputImage, outputs=x14)
m15 = Model(inputs=inputImage, outputs=x15)
m16 = Model(inputs=inputImage, outputs=x16)
m17 = Model(inputs=inputImage, outputs=x17)
m18 = Model(inputs=inputImage, outputs=x18)
m19 = Model(inputs=inputImage, outputs=x19)
m20 = Model(inputs=inputImage, outputs=x20)
m21 = Model(inputs=inputImage, outputs=x21)
m22 = Model(inputs=inputImage, outputs=x22)
m23 = Model(inputs=inputImage, outputs=x23)
m24 = Model(inputs=inputImage, outputs=x24)
m25 = Model(inputs=inputImage, outputs=x25)
m26 = Model(inputs=inputImage, outputs=x26)
m27 = Model(inputs=inputImage, outputs=x27)
m28 = Model(inputs=inputImage, outputs=x28)
m29 = Model(inputs=inputImage, outputs=x29)
m30 = Model(inputs=inputImage, outputs=x30)
m31 = Model(inputs=inputImage, outputs=x31)
m32 = Model(inputs=inputImage, outputs=x32)
m33 = Model(inputs=inputImage, outputs=x33)
m34 = Model(inputs=inputImage, outputs=x34)
m35 = Model(inputs=inputImage, outputs=x35)
m36 = Model(inputs=inputImage, outputs=x36)
#m37 = Model(inputs=inputImage, outputs=x37)
#m38 = Model(inputs=inputImage, outputs=x38)
#m39 = Model(inputs=inputImage, outputs=x39)

model.summary()
encoder.summary()
m1.summary()
m2.summary()
##########################################

##########################################
LEARNING_RATE = 0.0005 #2 times smaller than default 0.001
n_epochs = 20
batch_size = parts
opt = Adam(lr = LEARNING_RATE)

tf.config.run_functions_eagerly(True)

def r_loss(y_true, y_pred):
    y_true = tf.reshape(y_true, shape=(1, (parts*288*360*3)))
    y_pred = tf.reshape(y_pred, shape=(1, (parts*288*360*3)))

    idx_keep = tf.where(y_true[0,:]!=0)[:,-1] #idx_keep shape is (N, ) where N is +ve elements in 128*288*360*3 = 39813120 total elements
    t_true_keep = tf.gather(y_true[0,:], idx_keep) #t_true_keep shape is (N, )
    t_pred_keep = tf.gather(y_pred[0,:], idx_keep) #t_pred_keep shape is (N, )

    return K.sum(K.square(t_true_keep - t_pred_keep), axis = [0]) / parts

model.compile(optimizer=opt, loss = r_loss)

version = "v1_1_modifiedloss"
if not os.path.exists(version):
  os.makedirs("./"+version)

checkpoint_model = ModelCheckpoint(os.path.join(version, '/weights.h5'), save_weights_only = True, verbose=1)

history = model.fit(dataset_train, epochs=n_epochs, initial_epoch = 0, batch_size=batch_size,# steps_per_epoch=100, 
                    shuffle=True,
                    validation_data=dataset_val,
                    callbacks = [
                        checkpoint_model,
                        EarlyStopping(monitor='val_loss', patience=5, verbose=1, min_delta=0.0001),
                        ReduceLROnPlateau(monitor='val_loss', factor=0.1, patience=2, verbose=1),
                        TerminateOnNaN()])

model.save(version+'/model_'+version)
encoder.save(version+'/encoder_'+version)
m1.save(version+'/m1_'+version)
m2.save(version+'/m2_'+version)
m3.save(version+'/m3_'+version)
m4.save(version+'/m4_'+version)
m5.save(version+'/m5_'+version)
m6.save(version+'/m6_'+version)
m7.save(version+'/m7_'+version)
m8.save(version+'/m8_'+version)
m9.save(version+'/m9_'+version)
m10.save(version+'/m10_'+version)
m11.save(version+'/m11_'+version)
m12.save(version+'/m12_'+version)
m13.save(version+'/m13_'+version)
m14.save(version+'/m14_'+version)
m15.save(version+'/m15_'+version)
m16.save(version+'/m16_'+version)
m17.save(version+'/m17_'+version)
m18.save(version+'/m18_'+version)
m19.save(version+'/m19_'+version)
m20.save(version+'/m20_'+version)
m21.save(version+'/m21_'+version)
m22.save(version+'/m22_'+version)
m23.save(version+'/m23_'+version)
m24.save(version+'/m24_'+version)
m25.save(version+'/m25_'+version)
m26.save(version+'/m26_'+version)
m27.save(version+'/m27_'+version)
m28.save(version+'/m28_'+version)
m29.save(version+'/m29_'+version)
m30.save(version+'/m30_'+version)
m31.save(version+'/m31_'+version)
m32.save(version+'/m32_'+version)
m33.save(version+'/m33_'+version)
m34.save(version+'/m34_'+version)
m35.save(version+'/m35_'+version)
m36.save(version+'/m36_'+version)
#m37.save(version+'/m37_'+version)
#m38.save(version+'/m38_'+version)
#m39.save(version+'/m39_'+version)

np.save(version+'/history_'+version+'.npy', model.history.history)
##########################################
