extern "C" {
#include <GL/glut.h>  
#include <GL/gl.h>	
#include <GL/glu.h>	
#include <unistd.h>  
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

}


#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <map>
#include "glm/glm.hpp"

/***************/
#include <png.h>
/****************/


/* ASCII code for the escape key. */
#define ESCAPE 27
#define LIFE_MAX 3
#define XMIN -1000.0f
#define XMAX 1000.0f
#define YMIN -1000.0f
#define YMAX 1000.0f
#define BUFFERLEN 256
#define ERR_POS 5
#define ERR_ANGLE 5



/********************************************************************/
/*  Bout de code récupéré sur internet pour charger les textures    */
/********************************************************************/

/********************************************************************/

class TextureLoader
{
public:
    TextureLoader() {}

    static GLuint png(const char * file_name, int * width, int * height)
    {
        png_byte header[8];

        FILE *fp = fopen(file_name, "rb");
        if (fp == 0)
        {
            perror(file_name);
            return 0;
        }

        // read the header
        fread(header, 1, 8, fp);

        if (png_sig_cmp(header, 0, 8))
        {
            fprintf(stderr, "error: %s is not a PNG.\n", file_name);
            fclose(fp);
            return 0;
        }

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr)
        {
            fprintf(stderr, "error: png_create_read_struct returned 0.\n");
            fclose(fp);
            return 0;
        }

        // create png info struct
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
            fprintf(stderr, "error: png_create_info_struct returned 0.\n");
            png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
            fclose(fp);
            return 0;
        }

        // create png info struct
        png_infop end_info = png_create_info_struct(png_ptr);
        if (!end_info)
        {
            fprintf(stderr, "error: png_create_info_struct returned 0.\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
            fclose(fp);
            return 0;
        }

        // the code in this if statement gets called if libpng encounters an error
        if (setjmp(png_jmpbuf(png_ptr))) {
            fprintf(stderr, "error from libpng\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
            fclose(fp);
            return 0;
        }

        // init png reading
        png_init_io(png_ptr, fp);

        // let libpng know you already read the first 8 bytes
        png_set_sig_bytes(png_ptr, 8);

        // read all the info up to the image data
        png_read_info(png_ptr, info_ptr);

        // variables to pass to get info
        int bit_depth, color_type;
        png_uint_32 temp_width, temp_height;

        // get info about png
        png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
            NULL, NULL, NULL);

        if (width){ *width = temp_width; }
        if (height){ *height = temp_height; }

        // Update the png info struct.
        png_read_update_info(png_ptr, info_ptr);

        // Row size in bytes.
        int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

        // glTexImage2d requires rows to be 4-byte aligned
        rowbytes += 3 - ((rowbytes-1) % 4);

        // Allocate the image_data as a big block, to be given to opengl
        png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
        if (image_data == NULL)
        {
            fprintf(stderr, "error: could not allocate memory for PNG image data\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
            fclose(fp);
            return 0;
        }

        // row_pointers is for pointing to image_data for reading the png with libpng
        png_bytep * row_pointers = (png_bytep *)malloc(temp_height * sizeof(png_bytep));
        if (row_pointers == NULL)
        {
            fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
            free(image_data);
            fclose(fp);
            return 0;
        }

        // set the individual row_pointers to point at the correct offsets of image_data
        int i;
        for (i = 0; i < temp_height; i++)
        {
            row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
        }

        // read the png into image_data through row_pointers
        png_read_image(png_ptr, row_pointers);

        // Generate the OpenGL texture object
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, temp_width, temp_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glEnable(GL_GENERATE_MIPMAP);
//        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//        glGenerateMipmap(GL_TEXTURE_2D);
//        glBindTexture(GL_TEXTURE_2D, 0);

        // clean up
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        free(row_pointers);
        fclose(fp);
        return texture;
    }
};

/********************************************************************/
/*                 Fin du bout de code pas à moi                    */
/********************************************************************/

/********************************************************************/



class Tank
{
public:
	float   x;
	float   y;
	float   angle;
    float   velocity;
    int   life;
	
public:

	// constructeur
	Tank() {
        x = y = angle = velocity = 0.f;
        life = LIFE_MAX;
    }
   
    float getVelocity() {
      return velocity;
    }

    void setVelocity(float velocity) {
      this->velocity = velocity;
    }

    // met à jour la position du vaisseau (absolu)
    void setTranslation(float x, float y) {
      this->x = x;
      this->y = y;
    }

    // donne l'orientation du vaisseau
    void setOrientation(float angle) {
      this->angle = angle;
    }

    void accelerate() {    
        velocity += 10.f;
        if (velocity > 100.0f) velocity = 100.0f;
    }
    void decelerate() {
        velocity -= 10.f;
        if (velocity < -100.0f) velocity = -100.0f;
    }
	void turnRight() {
        angle -= 5.0f;
    }
	void turnLeft() {
        angle += 5.0f;
	}

  
    // met à jour le tank en prenant en compte la vitesse de celui-ci
    void update(double time) {
        if (velocity != 0.0f) {
            x += velocity*time*cos(angle*M_PI/180);        
            y += velocity*time*sin(angle*M_PI/180);

            if (x < XMIN) x = XMIN;
            if (x > XMAX) x = XMAX;
            if (y < YMIN) y = YMIN;
            if (y > YMAX) y = YMAX;
        }
    }
 
	
    void drawGL(bool local=true) {
        if(local) {
            glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(12.f, 0.f, -4.f);
            glRotatef(-angle, 0.0f, 0.0f, 1.0f);
            glTranslatef(-x, -y, 0.f);  
	    
        }

        glPushMatrix();
        glTranslatef(x,y,0.0);
        glRotatef(angle, 0.f, 0.f, 1.0f);
	

        if (local) glColor3f(0.1f, 0.1f, 0.f);
        else glColor3f(0.1f, 0.2f, 0.f);

        // Note : cette facon de coder (en faisant des #include de code C)
        // n'est pas recommandee... (il vaudrait mieux definir des fonctions
        // dans les fichiers .c et les appeler :-)
        #include "barrel.c"
        #include "ltread.c"
        #include "rtread.c"
        glColor3f(0.5f, 0.5f, 0.f);
        #include "body.c"
        glColor3f(0.3f, 0.3f, 0.f);
        #include "turret.c"
        glPopMatrix();
    }
  
};

class World
{
private:
    Tank*       localTank;
    Tank*       ghost;
    int         idLocal;
    double      lastTime;
    int         sock;
    struct sockaddr     *adrDest;
    int     longueurAdr;


    std::map<int ,Tank > tanks;


public:
    GLuint grass;
    GLuint brick;

	World() {
        lastTime = -1;
        localTank = new Tank();
        ghost = new Tank();

        int n;
        struct addrinfo	hints, *res, *ressave;



        // Récupération de l'adresse de destination (@IP+port)
        bzero(&hints, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC; // le système choisira IPv4 ou IPv6
        hints.ai_socktype = SOCK_DGRAM; // on veut UDP
        // à la place de localhost on peut mettre le nom de la machine
        // ou son adresse IP (v4 ou v6)
        if ( (n = getaddrinfo("localhost", "8123", &hints, &res)) != 0) {
            printf("erreur getaddrinfo : %s\n", gai_strerror(n));
            exit (1);
        }
        ressave = res;

        do { // Construction d'un socket compatible avec cette adresse
            sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if (sock >= 0)
                break;		// ça a marché
        } while ( (res = res->ai_next) != NULL);

        // aucune adresse n'a fonctionné, errno est positionné par socket()
        if (res == NULL) {
            perror("erreur socket");
            exit(1);
        }

        // on alloue puis on recopie l'adresse IP + port de destination
        adrDest = (struct sockaddr*)malloc(res->ai_addrlen);
        memcpy(adrDest, res->ai_addr, res->ai_addrlen);
        longueurAdr = res->ai_addrlen;
        // on libère la mémoire allouée par getaddrinfo
        freeaddrinfo(ressave);


        char buf[BUFFERLEN];        // Tampon pour le message
        sprintf(buf, "I");  // On écrit le message dans le tampon
         // Envoie le message
        if (sendto(sock, buf, strlen(buf) + 1, 0, adrDest, longueurAdr) <0) {
              perror("sendto");
              exit (1);
        }






    /*reception de notre id*/

        char message[1024];
        int cnt = recv(sock, message, sizeof(message),0);
        if (cnt < 0) {
             perror("erreur de recv");
             exit(1);
        }
        memcpy(&idLocal, message, sizeof(int));



        /*mise à jour du Ghost*/

        ghost->setOrientation(localTank->angle);
        ghost->setTranslation(localTank->x, localTank->y);
        ghost->setVelocity(localTank->getVelocity());

        ///on envoie au serveur où on est///


        sprintf(buf, "D %d %f %f %f %f", idLocal, ghost->x, ghost->y, ghost->angle, ghost->getVelocity());  // On écrit le message dans le tampon


        // Envoie le message
        if (sendto(sock, buf, strlen(buf) + 1, 0, adrDest, longueurAdr) <0) {
              perror("sendto");
              exit (1);
        }
    }
    


    Tank& getLocalTank() {
        return *localTank;
    }
 
  
    // met à jour le monde
    void update() {
        struct timeval date; //tps d'expiration
        gettimeofday(&date, NULL);
        double  time = date.tv_sec + date.tv_usec / 1000000.0;
        
        if (lastTime != -1) {
            localTank->update(time - lastTime);
            /*mise à jour du Ghost*/
            ghost->update(time - lastTime);
        }

        for(std::map<int,Tank> ::iterator it = tanks.begin(); it!=tanks.end(); ++it){
            if(idLocal != it->first)
                it->second.update(time - lastTime);
        }

        lastTime = time;      

        #define BUFFERLEN 256
        char buf[BUFFERLEN];        // Tampon pour le message

        /****************/
        /*DEAD RECKONING*/
        /****************/






        /*calcul d'erreur*/
        float err_x = abs( abs(localTank->x) - abs(ghost->x) );
        float err_y = abs( abs(localTank->y) - abs(ghost->y) );
        float err_alpha = abs( abs(localTank->angle) - abs(ghost->angle) );


        if ((err_alpha > ERR_ANGLE) || (err_x > ERR_POS) || (err_y > ERR_POS)){

            /*mise à jour du Ghost*/

            ghost->setOrientation(localTank->angle);
            ghost->setTranslation(localTank->x, localTank->y);
            ghost->setVelocity(localTank->getVelocity());

            ///on envoie au serveur où on est///


            sprintf(buf, "D %d %f %f %f %f", idLocal, ghost->x, ghost->y, ghost->angle, ghost->getVelocity());  // On écrit le message dans le tampon



            // Envoie le message
            if (sendto(sock, buf, strlen(buf) + 1, 0, adrDest, longueurAdr) <0) {
                  perror("sendto");
                  exit (1);
            }
        }



        /********************/
        /*END DEAD RECKONING*/
        /********************/




        fd_set ens;
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 50000; //attendre au max pendant 50ms = 50000us

        FD_ZERO(&ens);
        FD_SET(sock,&ens);

        while (select(sock+1, &ens, NULL, NULL, &timeout)>0){ //tous les timeout on verifie qu'il y ai rien de new dans ens (ui est le sock bah ouai)

            //recevoir message et traiter = creer nouveau tank, mettre à jour => map de tank, après on accedera à l'id du tank pour mettre à jour position etc
           int cnt;
           char message[1024];

           // on recupere le message
           cnt = recv(sock, message, sizeof(message),0);
           if (cnt < 0) {
                perror("erreur de recv");
                exit(1);
           }
           // on traite le message ici



           int id, idCible;
           float x,y, angle, vitesse;


           /****************/
           /*DEAD RECKONING*/
           /****************/
           /// quelqu'un se déplace
           if(sscanf(message, "D %d %f %f %f %f", &id, &x, &y, &angle, &vitesse )==5) {
               if (id != idLocal){

                    tanks[id].setTranslation(x,y);
                    tanks[id].setOrientation(angle);
                    tanks[id].setVelocity(vitesse);
                }

           }
           /********************/
           /*END DEAD RECKONING*/
           /********************/


           ///quelqu'un tire
           else if(sscanf(message, "S %d %d", &id, &idCible)==2) {

               if (idCible == idLocal){ /*on s'est fait tirer dessus!*/

                    //maj life
                    localTank->life--;

                    /*check de la vie*/
                    std::cout<<"vie = "<< localTank->life<<std::endl;

                    if (localTank->life == 0){

                        ///envoyer à tout le monde qu'on est mort///

                        sprintf(buf, "E %d", idLocal);  // On écrit le message dans le tampon

                        // Envoie le message
                        if (sendto(sock, buf, strlen(buf) + 1, 0, adrDest, longueurAdr) <0) {
                              perror("sendto");
                              exit (1);
                        }

                        exit(0);

                    }
                }
           }


           ///quelqu'un est mort
           else if(sscanf(message, "E %d", &idCible)== 1){
               /* effacer le mort de notre ecran et de notre liste de tank*/
               tanks.erase(idCible);

           }
        }
    }


    //tire sur le premier tank en face
    void shoot() {

        int idCible = -1;
        float alpha, dist;
        float distMin = 50000;

        //iterateur qui parcours la map
        for(std::map<int,Tank> ::iterator it = tanks.begin(); it!=tanks.end(); ++it){

            if (it->first != idLocal){ /*on s'assure de ne pas se tirer dessus*/

                //test de l'angle

                alpha = glm::degrees(atan2(it->second.y - localTank->y, it->second.x - localTank->x));

                std::cout<<"alpha = "<<alpha<<" et angle = "<<localTank->angle<<std::endl;

                if (abs(abs(alpha)-abs(localTank->angle)) < 2.5){


                    //calculer la distance entre les deux tank et comparer avec distMin
                    dist = sqrt((it->second.x - localTank->x) * (it->second.x - localTank->x) + (it->second.y - localTank->y) * (it->second.y - localTank->y));

                    if ((dist < distMin) && (dist !=0)){
                        distMin = dist;
                        idCible = it->first;
                    }
                }
            }
        }

        //tirer si il y a un tank dans le champ de vision
        if (idCible !=-1){

            char buf[BUFFERLEN];        // Tampon pour le message
            sprintf(buf, "S %d %d", idLocal, idCible);  // On écrit le message dans le tampon

            std::cout<<idLocal<<" tire sur "<<idCible<<std::endl;


            // Envoie le message
            if (sendto(sock, buf, strlen(buf) + 1, 0, adrDest, longueurAdr) <0) {
                  perror("sendto");
                  exit (1);
            }
        }

    }


        // Dessine un sol
        void drawFloor()
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, grass);

            glBegin(GL_QUADS);
                //glColor3f(0.3f, 0.7f, 0.3f);
                glTexCoord2d(0,60);  glVertex3f(-1000.0f,  1000.0f, 0.0f);
                glTexCoord2d(0,0);  glVertex3f( 1000.0f,  1000.0f, 0.0f);
                glTexCoord2d(60,0);  glVertex3f( 1000.0f, -1000.0f, 0.0f);
                glTexCoord2d(60,60);  glVertex3f(-1000.0f, -1000.0f, 0.0f);
            glEnd();

            glDisable(GL_TEXTURE_2D);
        }
 
 
        // Dessine le décors : des pyramides
        void drawScenery()
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, brick);

          for(int i = 0; i < 10; i++) {
            for(int j = 0; j < 10; j++) {
              glPushMatrix();
              glTranslatef(i*200.f - 900.f, j*200.f - 1000.f, 0.f);
              glScalef(10.f, 10.f, 10.f);
              glColor3f(0.7f, 0.5f, 0.0f);
              glBegin(GL_POLYGON);
                glTexCoord2d(1,2); glVertex3f(0.0f, 0.0f, 1.0f);        //Top Of Triangle (Front)
                glTexCoord2d(0,0); glVertex3f(-1.0f,1.0f, -1.0f);      //Left Of Triangle (Front)
                glTexCoord2d(2,0); glVertex3f(1.0f, 1.0f, -1.0f);       //Right Of Triangle (Front)

                glTexCoord2d(1,2); glVertex3f(0.0f, 0.0f, 1.0f);        //Top Of Triangle (Right)
                glTexCoord2d(0,0); glVertex3f(1.0f, 1.0f, -1.0f);       //Left Of Triangle (Right)
                glTexCoord2d(2,0); glVertex3f(1.0f, -1.0f, -1.0f);      //Right Of Triangle (Right)

                glTexCoord2d(1,2); glVertex3f(0.0f, 0.0f, 1.0f);        //Top Of Triangle (Back)
                glTexCoord2d(0,0); glVertex3f(1.0f, -1.0f, -1.0f);      //Left Of Triangle (Back)
                glTexCoord2d(2,0); glVertex3f(-1.0f, -1.0f, -1.0f);     //Right Of Triangle (Back)

                glTexCoord2d(1,2); glVertex3f(0.0f, 0.0f, 1.0f);        //Top Of Triangle (Left)
                glTexCoord2d(0,0); glVertex3f(-1.0f, -1.0f, -1.0f);     //Left Of Triangle (Left)
                glTexCoord2d(2,0); glVertex3f(-1.0f, 1.0f, -1.0f);      //Right Of Triangle (Left)
              glEnd();
              glPopMatrix();
            }
          }
           glDisable(GL_TEXTURE_2D);
        } 
        
         
  
      //dessine la jauge de vie
        void drawLife(){
	  
	  float ray = 0.00002;
	  
	  float z = 0.001;
	  
	  float x = 0.00052; 
	  
	  float espace = ray*3;
	  
	  float y = 0.00038;
	  
	   
	    glBegin(GL_POLYGON);
	    
		glColor3f(0.7f, 0.0f, 0.0f);	
				
	      
		for (int i =0; i<10; i++){
		  float angle = 2*M_PI*i /10;
		  glVertex3f(x + cos(angle)* ray , y + sin(angle) * ray, -z);
		}
		
	    glEnd();
		
	    
	    if (localTank->life > 1 ){	
	      
	      glBegin(GL_POLYGON);
	      
		glColor3f(0.7f, 0.0f, 0.0f);	      
		for (int i =0; i<10; i++){
		  float angle = 2*M_PI*i /10;
		  glVertex3f(x - espace + cos(angle)* ray , y + sin(angle) * ray, -z);
		}
	      glEnd();
	      
	      
	      
	      if (localTank->life > 2 ){
		glBegin(GL_POLYGON);
	      
		  glColor3f(0.7f, 0.0f, 0.0f);			
		    for (int i =0; i<10; i++){
		      float angle = 2*M_PI*i /10;
		      glVertex3f(x - espace * 2 + cos(angle)* ray , y + sin(angle) * ray, -z);
		    }
		  glEnd();
		}		  
	    }
	}
        
  
  
    // dessine le monde
    void drawGL() {
        localTank->drawGL(true);
        //ghost->drawGL(false);
        drawFloor();
        drawScenery();

        //iterateur qui parcours la map et drawgl(false) de tous
        for(std::map<int,Tank> ::iterator it = tanks.begin(); it!=tanks.end(); ++it){
                it->second.drawGL(false);
        }

    }



  
};


/////////////// Le Main et les fonctions callback de la GLUT /////////////////

World *TheWorld = NULL;



/* The number of our GLUT window */
int window; 

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void InitGL(int Width, int Height)	        /* We call this right after our OpenGL window is created. */
{
  glClearColor(0.2f, 0.2f, 0.7f, 0.0f);		/* This Will Clear The Background Color To Black */
  glClearDepth(1.0);				/* Enables Clearing Of The Depth Buffer */
  glDepthFunc(GL_LESS);				/* The Type Of Depth Test To Do */
  glEnable(GL_DEPTH_TEST);			/* Enables Depth Testing */
  glShadeModel(GL_SMOOTH);			/* Enables Smooth Color Shading */

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();				/* Reset The Projection Matrix */

  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.001f,10000.0f);	/* Calculate The Aspect Ratio Of The
  Window*/

  glMatrixMode(GL_MODELVIEW);


  /*********************************/

  /*********************************/
}

/* The function called when our window is resized (which shouldn't happen, because we're fullscreen) */
void ReSizeGLScene(int Width, int Height)
{
  if (Height==0)				/* Prevent A Divide By Zero If The Window Is Too Small */
    Height=1;

  glViewport(0, 0, Width, Height);		/* Reset The Current Viewport And Perspective Transformation */

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.001f,10000.0f);
  glMatrixMode(GL_MODELVIEW);
}

/* The main drawing function. */
void DrawGLScene()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		/* Clear The Screen And The Depth Buffer */
  
  glLoadIdentity();				/* Reset The View */
  
  TheWorld->drawLife();

  TheWorld->drawGL();
  

  /* swap buffers to display, since we're double buffered.*/
  glutSwapBuffers();
}



/* The function called whenever a key is pressed. */
void keyPressed(unsigned char key, int x, int y) 
{
    /*shoot*/
    if (key =='t') TheWorld->shoot();

    /* If escape is pressed, kill everything. */
    if (key == ' ') TheWorld->getLocalTank().setVelocity(0.0f);
    else if (key == ESCAPE) 
    { 
	/* shut down our window */
	glutDestroyWindow(window); 
	
	/* exit the program...normal termination. */
	exit(0);                   
    }
}

void specialFunc(int key, int x, int y)  
{  
	switch (key) {	
		case GLUT_KEY_UP: // accelerate
            TheWorld->getLocalTank().accelerate();
			break;
		case GLUT_KEY_DOWN: // decelerate
            TheWorld->getLocalTank().decelerate();
			break;
		case GLUT_KEY_RIGHT: // turn right
            TheWorld->getLocalTank().turnRight();
			break;
		case GLUT_KEY_LEFT: // turn left
            TheWorld->getLocalTank().turnLeft();
			break;
		default: // do nothing
			break;
	}
}


void simulation() {
    TheWorld->update();
    glutPostRedisplay();
}



int main(int argc, char **argv) 
{  
    TheWorld = new World();

  
  /* Initialize GLUT state - glut will take any command line arguments that pertain to it or 
     X Windows - look at its documentation at http://reality.sgi.com/mjk/spec3/spec3.html */  
  glutInit(&argc, argv);  

  /* Select type of Display mode:   
     Double buffer 
     RGBA color
     Depth buffer */  
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);  

  /* get a 640 x 480 window */
  glutInitWindowSize(640, 480);  

  /* the window starts at the upper left corner of the screen */
  glutInitWindowPosition(0, 0);  

  /* Open a window */  
  window = glutCreateWindow("Tank");  

  /* Register the function to do all our OpenGL drawing. */
  glutDisplayFunc(&DrawGLScene);  

  /* Go fullscreen.  This is the soonest we could possibly go fullscreen. */
  /* glutFullScreen(); */

  /* Even if there are no events, redraw our gl scene. */
  /* glutIdleFunc(&DrawGLScene); */

  /* Register the function called when our window is resized. */
  glutReshapeFunc(&ReSizeGLScene);

  /* Register the function called when the keyboard is pressed. */
  glutKeyboardFunc(&keyPressed);
  glutSpecialFunc(specialFunc);
  
  glutIdleFunc(simulation);

  /* Initialize our window. */
  InitGL(640, 480);

  /*chargement de texture*/


  int h, l;
  TheWorld->grass = TextureLoader::png("textures/grasslight-big.png",&h , &l);

  TheWorld->brick = TextureLoader::png("textures/brique.png",&h , &l);
  
  /* Start Event Processing Engine */  
  glutMainLoop();  

  return 1;
}
