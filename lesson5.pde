import ddf.minim.*;

Minim minim;
AudioPlayer player;


void setup(){
  size(700, 700);
  minim = new Minim(this);
  player = minim.loadFile("click.mp3");
  background(255,255,255);
}

int size_width = 700;
int x1 = 100, x2;
int y = 350;

int direction = 1;
int count = 0;

void draw(){
  background(255,255,255);
  
  if(x1 > 325){
    direction = -1;
  }else if(x1 < 100){
    direction = 1;
  }
  
  if(direction == 1){
    x1 = x1 + 5;
    x2 = size_width - x1;
  }else if(direction == -1){
    x1 = x1 - 5;
    x2 = size_width - x1;
  }
  
  // sound
 
 /*
  if(x1 == 330){
    player.rewind();
    player.play();
  }
  */
  
  if(x1 == 330){
    count++;
  }
  
  
  //change color
  if(count%2 == 1){
    fill(255, 0, 0);
    ellipse(x1, y, 50, 50);
    
    fill(0, 0, 255);
    ellipse(x2, y, 50, 50);
  }else if(count%2 == 0){
    fill(0, 0, 255);
    ellipse(x1, y, 50, 50);
    
    fill(255, 0, 0);
    ellipse(x2, y, 50, 50);
  }
  
}
