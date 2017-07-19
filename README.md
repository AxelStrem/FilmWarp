# FilmWarp [![Build status](https://ci.appveyor.com/api/projects/status/gtmiqnihr8b185tx?svg=true)](https://ci.appveyor.com/project/AxelStrem/filmwarp)
The tool to morph & distort videos  

![ezgif-1-a7d547e44e](https://user-images.githubusercontent.com/11349690/28335444-167668fe-6c07-11e7-8a37-43b5a5b2f1b8.gif)

### Download & Requirements

- [Win64 executable](https://sourceforge.net/projects/filmwarp/files/FilmWarp.exe/download)
- requires [opencv](http://opencv.org/releases.html); ([Prebuilt Win64 binaries](https://sourceforge.net/projects/filmwarp/files/opencv.zip/download))

### Calling Syntax

    FilmWarp <input file> <output file> <morph expression> [optional parameters]

- `<input file>` - path to the source video file
- `<output file>` - path to the resulting video or image file
- `<morph expression>` - mathematical expression that defines the transformation

### Examples

- vertical flip: `FilmWarp in.mp4 out.mp4 [x;h-y;z]`  
![ezgif-1-790f38b76f](https://user-images.githubusercontent.com/11349690/28335541-696e0bd4-6c07-11e7-9662-996020434474.gif)

- rolling shutter: `FilmWarp in.mp4 out.mp4 [x;y;z-y*0.1]`
- cells: `FilmWarp in.mp4 out.mp4 [(4*x)#w;(4*y)#h;z]`
- reverse: `FilmWarp in.mp4 out.mp4 [x;y;l-z]`

