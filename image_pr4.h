// following functions are needed for ppm generation:
// - draw circle(x,y,R,r,g,b) (sun)
// - draw background
// - draw object (duck). Object ia another ppm image
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// execute system command
std::string exec_cust(std::string cmd){
	char buffer[1024];
	std::string result = "";
    FILE* pipe = popen(cmd.c_str(), "w");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;	
}

struct RGB{
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct Point2Di{
    int x;
    int y;
};


struct ImagePPM{
	// pixels
	int width, height;
	int n_bytes;
	char* data;
} image;

// create new ppm image
int make_image(int w,int h){
	image.width = w;
	image.height = h;
    image.n_bytes =  image.width*image.height*3;
    image.data = new char[image.n_bytes];
    return 0;
}

// returns color component (color==0 -red,color==1-green,color==2-blue
// color == 3 - luminocity
// for pixel located at (row,column)
unsigned char get_pixel( ImagePPM image,int row,int col, int color)
{
    // calculate address in 1D array of pixels
    int address = image.width*row*3 + col*3;  // standard ppm
    //int address = image.width*row*4 + col*4;
    if ((row < 0 ) || (row > image.height) )  {
        std::cout<<"row is out of range\n";
        return -1;
    }
    if ( (col< 0) || (col > image.width))    {
        std::cout<<"column is out of range\n";
        return -1;
    }
    unsigned char r = image.data[address];
    unsigned char g = image.data[address+1];
    unsigned char b = image.data[address+2];
//std::cout<<"GP "<<(int)r<<" "<<(int)g<<" "<<(int)b<<std::endl;		          
    if (color==0) { return r;}
    if (color==1) { return g;}
    if (color==2) { return b;}
    if (color==3)    {
		int yi = r+g+b;
        unsigned char y = yi/3;
//std::cout<<"GP y="<<(int)y<<std::endl;		          
        
        return y;
    }
    printf("Color encoding wrong: 0-red, 1-green,2-blue,3 - luminosity\n");
    return -2; //error
}

int set_pixel(ImagePPM& image, int row, int col,
              unsigned char red,unsigned char green,unsigned char blue)
{
    //int address = image.width*row*4 + col*4; // RPI
    int address = image.width*row*3 + col*3;  // standard ppm
    if ((address < 0) || (address>image.width*image.height*3))
    {
        printf("SetPixel(): wrong x,y coordinates\n");
        printf(" row= %d col=%d address=%d\n",row,col,address);
        printf(" width= %d height=%d\n",image.width,image.height);
        return -1;
    }
    image.data[address] = red;
    image.data[address+1] = green;
    image.data[address+2] = blue;
    return 0;
}

int SavePPMFile(std::string filename, ImagePPM& image){
	std::ofstream fout;
	fout.open(filename,std::ofstream::binary);
	if (!fout.is_open()){ std::cout<<"no file"; return -1;}
	fout<<"P6"<<std::endl;
	fout<<(int)image.width<<" "<<(int)image.height<<" 255"<<std::endl;
	fout.write(image.data,image.n_bytes);
	flush(fout);
	fout.close();
	return 0;
}

int save_bmp_file(std::string filename, ImagePPM& image){
	//Stream & out;
	std::ofstream fout;
	fout.open(filename,std::ofstream::binary);
	if (!fout.is_open()){ std::cout<<"no file"; return -1;}
	
	unsigned int w = image.width, h = image.height;
    unsigned int pad = w * -3 & 3;
    unsigned int total = 54 + 3*w*h + pad*h;
    unsigned int head[13] = {total, 0, 54, 40, w, h, (24<<16)|1};
    //char const* rgb = (char const*)img.rgb.data();

    fout.write("BM",2);
    fout.write((char*)head, 52);
    char* rgb = new char[3];
    for (int row = image.height ; row >= 0; row--)
    for (int col = 0 ; col < image.width ; col++){
		int address = image.width*row*3 + col*3;
		rgb[0] = image.data[address+2];;
		rgb[1] = image.data[address+1];;
		rgb[2] = image.data[address];;
		fout.write(rgb,3); 
	}
    
    fout.write(image.data,image.n_bytes);
	
    return 1;
}

int OpenPPMFile(std::string filename,ImagePPM& image){
	//std::cout<<"Opening single ppm file"<<std::endl;
	
	std::ifstream fin;
	fin.open(filename);
	if (!fin.is_open()){ std::cout<<"no file"; return -1;}
    // get first line - contains P6
    std::string fd1;
    getline(fin,fd1);
    //std::cout<<fd1<<std::endl; 
    // get secondline
    getline(fin,fd1);
    //std::cout<<fd1<<std::endl; 
    int depth;
    sscanf( fd1.c_str(),"%d %d %d\n",&image.width ,&image.height,&depth );
    // size of the image
    // parse string
    //std::cout<<"image.height = "<<image.height;
    //std::cout<<" image.width = "<<image.width<<std::endl;
    
    image.n_bytes =  image.width*image.height*3;
    image.data = new char[image.n_bytes];
    fin.read(image.data, image.n_bytes);
    fin.close();
    return 0;
}

// fill whole image
int draw_background(ImagePPM& image, char r,char g, char b){
	for (int col = 0 ; col < image.width ; col++)
	for (int row = 0 ; row < image.height; row++){
		set_pixel(image, row , col, r,g,b);
	} 
	return 0;
}

// draw circle
int draw_circle(ImagePPM& image, int xc, int yc, int R, char r,char g, char b){
	for (int col = xc-R ; col<xc+R ; col++)
	for (int row = yc-R; row < yc+R ; row++){
		int dx = col-xc;
		int dy = row -yc;
		if ((row>0) && (row < image.height ) &&
		    (col>0) && (col <image.width ) &&
		    ( dx*dx + dy*dy<R*R)) set_pixel(image,row,col,r,g,b);
	}
	return 0;
}


int draw_square(ImagePPM& image, int xc,int yc, int W, char r, char g, char b){
	int xtop = xc - W/2;
	int ytop = yc - W/2;
	int xbot = xc + W/2;
	int ybot = yc + W/2;
	for (int x = xtop ; x < xbot; x++) {
	  if ((x>=0)&&(x<image.width)&&(ytop>=0)&&(ytop<image.height))
	     set_pixel(image,ytop,x,r,g,b);
    }
	for (int x = xtop ; x < xbot; x++) {
	  if ((x>=0)&&(x<image.width)&&(ybot>=0)&&(ybot<image.height))
        set_pixel(image,ybot,x,r,g,b);
	}  
	for (int y = ybot ; y > ytop; y--){
	  if ((xtop>=0)&&(xtop<image.width)&&(y>=0)&&(y<image.height))
		 set_pixel(image,y,xtop,r,g,b);
	}
	for (int y = ybot ; y > ytop; y--) {
	  if ((xbot>=0)&&(xbot<image.width)&&(y>=0)&&(y<image.height))
		set_pixel(image,y,xbot,r,g,b);
	}
	return 0;
}


// pictures in the scene
// white (rgb>250) is left  with background color
struct Object {
	ImagePPM object_image;
	int load_from_file(std::string fn);
	double vx,vy;
	int x0,y0;
};

int Object::load_from_file(std::string fn){
	OpenPPMFile(fn,object_image);
	
	return 0;
}

void insert_object(ImagePPM& im, Object ob, int x, int y){
	
	for (int objrow = 0 ; objrow < ob.object_image.height; objrow++)
	for (int objcol = 0 ; objcol < ob.object_image.width; objcol++){
		unsigned char r = get_pixel(ob.object_image,objrow,objcol, 0);
		unsigned char g = get_pixel(ob.object_image,objrow,objcol, 1);
		unsigned char b = get_pixel(ob.object_image,objrow,objcol, 2);
		if (!((r>250)&&(g>250)&&(b>250))) // not perfect white
		set_pixel(im, y+objrow, x+objcol, r,g,b);
	}
}


struct Scene{
	RGB background;
	std::vector<Object> things;
	int x_sun,y_sun,r_sun;
	int time;
	int x_aim,y_aim;
	double omega;
	std::ofstream log_file;
} scene;


int set_scene(int level){

    scene.log_file.open ("log.txt");
    if(level>0){
		// add clouds
	  Object thing;
	  //scene.things.push_back(thing);
      thing.load_from_file("cloud1.ppm"); // 
	  thing.x0 = 100; thing.y0 = 570; 
	  thing.vx = 10; thing.vy = 0; 
	  scene.things.push_back(thing);
      
      thing.load_from_file("cloud1.ppm"); // 
	  thing.x0 = 300; thing.y0 = 770; 
	  thing.vx = 50; thing.vy = 0; 
	  scene.things.push_back(thing);

	  if (level == 2){ // add ducks
	        thing.load_from_file("duck3.ppm");
	        thing.x0 = 100; thing.y0 = 800; 
	        thing.vx = 40; thing.vy = 0; 
	        scene.things.push_back(thing);
	        thing.load_from_file("duck3.ppm");
	        thing.x0 = 200; thing.y0 = 700; 
	        thing.vx = -10; thing.vy = 0; 
	        scene.things.push_back(thing);
	        thing.load_from_file("duck3.ppm");
	        thing.x0 = 200; thing.y0 = 400; 
	        thing.vx = 50; thing.vy = 0; 
	        scene.things.push_back(thing);
	    
      }
    }
    scene.omega = 0.1;
    scene.x_aim = 450;
    scene.y_aim = 450;
    std::cout<<"Scene initialized.."<<std::endl;
    return 0;
}

int draw_all(int time){
	 // std::cout<<" Started drawing..."<<std::endl;
	// draw blue sky and sun on it
	  draw_background(image,120,220,255);
	  scene.x_sun = 450 + 200*cos(scene.omega*time);
	  scene.y_sun = 750 + 200*sin(scene.omega*time);
	  draw_circle(image, scene.x_sun, scene.y_sun, 14, 255,0,0);
	 // std::cout<<" draw "<< scene.things.size()<<" objects..."<<std::endl;
      for (Object thing:scene.things){
		//  std::cout<<".....object..."<<std::endl;
		 // std::cout<<"... image size is "<<thing.object_image.n_bytes<<std::endl;
		  if (thing.object_image.n_bytes>0)
	      insert_object(image,thing ,thing.x0+time*thing.vx,
	                                 thing.y0+time*thing.vy);
	  }	  
	  // draw aim as a square
	  draw_square(image,scene.x_aim,scene.y_aim,90,0,0,0);
	  //std::cout<<" scene prepared"<<std::endl;
	  //SavePPMFile("m1.ppm", image);
	  save_bmp_file("m1.bmp", image);
	  scene.log_file<<time<<" "<<scene.x_sun<<" "<<scene.y_sun;
	  scene.log_file<<" "<<scene.x_aim<<" "<<scene.y_aim<<std::endl;
	  scene.log_file.flush();
	  //std::cout<<"Image saved..."<<std::endl;
	return 0;
}

void get_aim(int& x, int& y){
	x = scene.x_aim;
	y = scene.y_aim;
}

void move_aim(double angle){
	
    std::cout<<"  Inside move x="<<scene.x_aim<<"  y="<<scene.y_aim<<std::endl;
    std::cout<<" angle = "<<angle*180/3.14159<<std::endl;
	scene.x_aim = scene.x_aim + 30.0*cos(angle);
	scene.y_aim = scene.y_aim + 30.0*sin(angle);
    std::cout<<"  Inside move x="<<scene.x_aim<<"  y="<<scene.y_aim<<std::endl;
}

void init(int level){
	if (make_image(900 , 900) == 0){
        set_scene(level);
        draw_all(level);
    } else {std::cout<<" Fail to reserve image memory"<<std::endl;}
}

