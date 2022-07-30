mkdir -p images;

echo "Converting jpgs";
for file in chat/photos/*.jpg;
do magick mogrify -format bmp $file &&
magick "${file%.*}.bmp" -alpha OFF -compress none -resize '480x320\>' "${file}_temp" &&
mv "${file}_temp" "${file%.*}.bmp";
done;

echo "Converting pngs";
for file in chat/photos/*.png;
do magick mogrify -format bmp $file &&
magick "${file%.*}.bmp" -alpha OFF -compress none -resize '480x320\>' "${file}_temp" &&
mv "${file}_temp" "${file%.*}.bmp";
done;
mv chat/photos/*.bmp images/;

echo "Converting gifs";
for file in chat/gifs/*.gif;
do magick convert "$file[0]" "${file%.*}.bmp" &&
magick "${file%.*}.bmp" -alpha OFF -compress none -resize '480x320\>' "${file}_temp" &&
mv "${file}_temp" "${file%.*}.bmp";
done;
mv chat/gifs/*.bmp images/

echo "Converting videos";
for file in chat/videos/*;
do magick convert "$file[0]" "${file%.*}.bmp" &&
magick "${file%.*}.bmp" -alpha OFF -compress none -resize '480x320\>' "${file}_temp" &&
mv "${file}_temp" "${file%.*}.bmp";
done;
mv chat/videos/*.bmp images/

echo "Finished!";

