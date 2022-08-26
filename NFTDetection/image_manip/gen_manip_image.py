import os
import np
import PIL
import PIL.Image
import PIL.ImageEnhance
import glob

def rotateImage(image_file):
        parts = image_file.split('\\')
        file_name = parts[len(parts)-1]
        try:
            img2 = PIL.Image.open(image_file)
            background_pixel = img2.getpixel((1, 1))
            img2 = img2.rotate(angle=-5,expand=False,fillcolor=background_pixel)
            img3 = img2.save(".\\rotated\\" + file_name)
        except Exception as e: 
            print(e)
            exit

def brightnessImage(image_file):
        parts = image_file.split('\\')
        file_name = parts[len(parts)-1]
        try:
            img2 = PIL.Image.open(image_file)
            enhancer = PIL.ImageEnhance.Brightness(img2)
            factor = 0.9 
            im_output = enhancer.enhance(factor)
            im_output.save(".\\brightness\\" + file_name)
        except Exception as e: 
            print(e)
            exit

def blurrimage(image_file):
        parts = image_file.split('\\')
        file_name = parts[len(parts)-1]
        try:
            img2 = PIL.Image.open(image_file)
            img2 = img2.filter(PIL.ImageFilter.GaussianBlur(radius = 2))
            img2.save(".\\blurr\\" + file_name)
        except Exception as e: 
            print(e)
            exit

def resizeImage(image_file):
        parts = image_file.split('\\')
        file_name = parts[len(parts)-1]
        try:
            img2 = PIL.Image.open(image_file)
            width, height = img2.size
            pct_resize = 0.90
            width_loss = (width - (int)(width * pct_resize)) / 2
            height_loss = (height - (int)(height * pct_resize))
            img2 = img2.crop((width_loss, height_loss, width - width_loss * 2, height))
            img2 = img2.resize((width, height))
            img2.save(".\\resize\\" + file_name)
        except Exception as e: 
            print(e)
            exit

def moveImage(image_file):
        parts = image_file.split('\\')
        file_name = parts[len(parts)-1]
        try:
            img2 = PIL.Image.open(image_file)
            width, height = img2.size
            background_pixel = img2.getpixel((1, 1))
            start_at_y = 0
            for i in range(0,height-1):
                pixel_now = img2.getpixel((width / 2, i))
                if pixel_now != background_pixel:
                    start_at_y = i
                    break;
            start_at_y = (int)(start_at_y / 2)
            img_high = img2.crop((0, 0, width, start_at_y))
            img_low = img2.crop((0, start_at_y, width, height))
            PIL.Image.Image.paste(img2, img_low, (0, 0))
            PIL.Image.Image.paste(img2, img_high, (0, height - start_at_y))
            img2.save(".\\move\\" + file_name)
        except Exception as e: 
            print(e)
            exit
            
def colorswap(image_file):
        parts = image_file.split('\\')
        file_name = parts[len(parts)-1]
        try:
            img2 = PIL.Image.open(image_file)
            background_pixel = img2.getpixel((1, 1))
            background_pixel_new = ((int)(background_pixel[0] * 0.9),(int)(background_pixel[1] * 0.9),(int)(background_pixel[2] * 0.9))
            width, height = img2.size
            for y in range(0,height):
                for x in range(0,width):
                    pixel_now = img2.getpixel((x, y))
                    if pixel_now == background_pixel:
                        img2.putpixel((x, y), background_pixel_new)
            img2.save(".\\colorswap\\" + file_name)
        except Exception as e: 
            print(e)
            exit
            
image_fileList = []
mainFolder = "..\\page_fetcher\\images\\cryptopunks\\"
folderList = [x[0] for x in os.walk(mainFolder)]
for folder in folderList:
    for image_file in glob.glob(folder + '/*.*'):
        if not os.path.isfile(image_file):
            continue
        print("handle image : " + image_file)
        rotateImage(image_file)
        brightnessImage(image_file)
        blurrimage(image_file)
        resizeImage(image_file)
        moveImage(image_file)
        colorswap(image_file)
#        os.exit
