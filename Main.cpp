/*
Trabaja con vídeo: adquisición en tiempo real
El código es el conseguido en Proyecto-Rostro2, que está completo para la detecci#on e identificación del rostro y sus características
*/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <tchar.h>
#include <opencv2\opencv.hpp>
#include <SDKDDKVer.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat rotate(Mat src, double angle)
{
	Mat dst;
	Point2f pt(src.cols / 2., src.rows / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(src, dst, r, Size(src.cols, src.rows));
	return dst;
}

struct caracteristicas {
	int x;		//centro.x
	int y;		//centro.y
	int ancho;
	int alto;
};
struct cara {
	caracteristicas CojoD;
	caracteristicas CojoI;
	caracteristicas Cnariz;
	caracteristicas Cboca;
	int perfil;			//0: cara frontal	1: perfil izq	2:perfil derecho
	int ang;			//angulo sobre eje z del rostro
};

int main()
{
	/****Variables auxiliares para el desarrollo y comprobación del codigo*****/
	int detectCara = 1; int procesIma = 1; int detectOjos = 1; int detectNariz = 1; int detectBoca = 1;
	int guardarIma = 0; int Reglas = 1;

	/****************************************************************/
	/***************  Comprobar el acceso a la cámara  **************/
	VideoCapture Vcap;
	if (!Vcap.open(1)) //0 : cámara integrada del ordenador. 1 : webcam externa
	{
		cout << "No se puede acceder a la Cámara." << endl;
		system("PAUSE");
		return -1;
	}

	/*************************************************************************************************************/
	/****************Detectores Viola-Jones de Características***************************************************/

	CascadeClassifier detector_cara;
	CascadeClassifier detector_ojos, detector_ojosIzq, detector_ojosDer;
	CascadeClassifier detector_boca, detector_boca1;
	CascadeClassifier detector_nariz, detector_nariz2, detector_nariz3;

	if (detectCara) {
		if (!detector_cara.load("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt.xml"))
		{
			cout << "No se puede abrir el clasificador de cara." << endl;
			system("PAUSE"); //Espera entrada de usuario por teclado
			return -1;
		}
	}
	if (detectOjos) {
		if (!detector_ojos.load("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_eye.xml"))
		{
			cout << "No se puede abrir el clasificador de ojos." << endl;
			system("PAUSE");
			return -1;
		}
		if (!detector_ojosIzq.load("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_lefteye_2splits.xml"))
		{
			cout << "No se puede abrir el clasificador de ojos izquierdos." << endl;
			system("PAUSE");
			return -1;
		}
		if (!detector_ojosDer.load("C:\\opencv\\sources\\data\\haarcascades\\haarcascade_righteye_2splits.xml"))
		{
			cout << "No se puede abrir el clasificador de ojos derechos." << endl;
			system("PAUSE");
			return -1;
		}
	}
	if (detectNariz) {
		if (!detector_nariz.load("C:\\Users\\AmandaGM\\Desktop\\TFM\\xml\\nariz.xml"))
		{
			cout << "No se puede abrir el clasificador de nariz." << endl;
			system("PAUSE");
			return -1;
		}
		if (!detector_nariz2.load("C:\\Users\\AmandaGM\\Desktop\\TFM\\xml\\FaceFeaturesDetectors\\Nariz.xml"))
		{
			cout << "No se puede abrir el clasificador de nariz2." << endl;
			system("PAUSE");
			return -1;
		}
		if (!detector_nariz3.load("C:\\Users\\AmandaGM\\Desktop\\TFM\\xml\\FaceFeaturesDetectors\\Nariz_nuevo_20stages.xml"))
		{
			cout << "No se puede abrir el clasificador de nariz3." << endl;
			system("PAUSE");
			return -1;
		}
	}
	if (detectBoca) {
		if (!detector_boca.load("C:\\Users\\AmandaGM\\Desktop\\TFM\\xml\\boca.xml"))
		{
			cout << "No se puede abrir el clasificador de boca." << endl;
			system("PAUSE");
			return -1;
		}
		if (!detector_boca1.load("C:\\Users\\AmandaGM\\Desktop\\TFM\\xml\\FaceFeaturesDetectors\\Mouth.xml"))
		{
			cout << "No se puede abrir el clasificador de boca1." << endl;
			system("PAUSE");
			return -1;
		}
	}


	/**********************************************************************
	         Bucle para la obtención de frames y su procesamiento  
	**********************************************************************/
	while (1)
	{
		int alfa = -70; int dist_ojos = 0; int perfil = 0;  //0: frontal	1:izquierda		2:derecha
		float factor = 0.0f;

		Mat imagen, im_gris, im_equa, im_bin;
		Mat im_rotate = imagen;
		Mat Rim_imagen, Rim_gris, Rim_equa;		

		Vcap.read(imagen);

		cvtColor(imagen, im_gris, CV_BGR2GRAY); //Guardamos en gris la imagen convertida a tonos grises
		equalizeHist(im_gris, im_equa);			//Ecualizamos la imagen para estandarizar el contraste

		/******* Detección de rostro ******/

		vector<Rect> vect_cara;					//Vector para guardar las características de los rostros encontrados
												//detector.detectMultiScale(im_gris, rect);  --> resultados parecidos a los obtenidos tras equalizar
		detector_cara.detectMultiScale(im_equa, vect_cara, 1.2, 3, 0, Size(60, 60));

		Mat aux;
		detector_cara.detectMultiScale(im_equa, vect_cara, 1.2, 3, 0, Size(60, 60));

		while (vect_cara.empty() && alfa < 55) {
			alfa = alfa + 15;
			im_rotate = rotate(im_equa, alfa);
			detector_cara.detectMultiScale(im_rotate, vect_cara, 1.2, 3, 0, Size(60, 60));
		}
		for (Rect rect : vect_cara)	//pintar rectangulo sobre la cara
		{
			if (alfa != -70)
			{
				imagen = rotate(imagen, alfa); im_gris = rotate(im_gris, alfa); im_equa = rotate(im_equa, alfa);
			}
			
			if (im_gris(rect).size().width > 70) {
				Rim_gris = im_gris(rect);
				Rim_equa = im_equa(rect);
				Rim_imagen = imagen(rect);
				rectangle(imagen,
					Point(rect.x, rect.y),
					Point(rect.x + rect.width, rect.y + rect.height),
					Scalar(0, 255, 0), 2);
			}
			else {
				rectangle(imagen,
					Point(rect.x, rect.y),
					Point(rect.x + rect.width, rect.y + rect.height),
					Scalar(0, 0, 255), 2); 
				cout << "Rostro reconocido muy pequeño para su procesamiento" << endl;}
		}

		if (vect_cara.empty()) {}
		else {
			/************* Ajustar el tamaño del rostro encontrado********************/
			if (procesIma) {
				factor = 400.0f / Rim_imagen.size().width;

				resize(Rim_imagen, Rim_imagen, Size(int(Rim_gris.cols * factor), int(Rim_gris.rows * factor)));
				resize(Rim_gris, Rim_gris, Size(int(Rim_gris.cols * factor), int(Rim_gris.rows * factor)));
				resize(Rim_equa, Rim_equa, Size(int(Rim_equa.cols * factor), int(Rim_equa.rows * factor)));
				resize(imagen, imagen, Size(int(imagen.cols * factor), int(imagen.rows * factor)));
			}

			/*************Si hay cara, se buscan otras caracteristicas**************/
			Mat im_ojos_gris, im_ojos_equa, im_ojos_bin, im_ojos_binE, im_ojos_thr;
			Mat im_boca_gris, im_boca_equa;
			Mat im_gris_nariz, im_equa_nariz;
			vector<caracteristicas> oj, ojD, ojI, bo, na;
			if (detectOjos) {
				int indO = 1; int ho = 0;
				vector<Rect> vect_ojos, vect_ojosIzq, vect_ojosDer, vect_ojosE, vect_ojosIzqE, vect_ojosDerE;		//Vector para guardar las características de los ojos encontrados

																													//im_ojos_gris = Rim_gris(Rect(0, Rim_gris.size().height / 8, Rim_gris.size().width, Rim_gris.size().height * 4 / 8));	namedWindow("O_gr", WINDOW_AUTOSIZE);	moveWindow("O_gr", 0, 300); imshow("O_gr", im_ojos_gris);
				ho = Rim_gris.size().height * 3 / 16;
				im_ojos_gris = Rim_gris(Rect(0, Rim_gris.size().height * 3 / 16, Rim_gris.size().width, Rim_gris.size().height * 6 / 16));	namedWindow("O", WINDOW_AUTOSIZE);	moveWindow("O", 500, 0); imshow("O", im_ojos_gris);
				im_ojos_equa = Rim_equa(Rect(0, Rim_equa.size().height * 3 / 16, Rim_equa.size().width, Rim_equa.size().height * 6 / 16));	namedWindow("O_eq", WINDOW_AUTOSIZE);	moveWindow("O_eq", 500, 250); imshow("O_eq", im_ojos_equa);
																													//	bilateralFilter(im_ojos_gris, im_ojos_bin, 15, 280, 80);	namedWindow("O_bin", WINDOW_AUTOSIZE);	moveWindow("O_bin", 900, 600); imshow("O_bin", im_ojos_bin);
				bilateralFilter(im_ojos_equa, im_ojos_binE, 15, 280, 80);	namedWindow("O_binE", WINDOW_AUTOSIZE);	moveWindow("O_binE", 1200, 600); imshow("O_binE", im_ojos_binE);
				cv::threshold(im_ojos_gris, im_ojos_thr, 0.65*sum(im_ojos_gris)[0] / (im_ojos_gris.cols*im_ojos_gris.rows), 255, CV_THRESH_BINARY); namedWindow("O2", WINDOW_AUTOSIZE);	moveWindow("O2", 500, 500); imshow("O2", im_ojos_thr);

				detector_ojos.detectMultiScale(im_ojos_gris, vect_ojos, 1.2, 3, 0, Size(60, 60), Size(120, 120));
				detector_ojosIzq.detectMultiScale(im_ojos_gris, vect_ojosIzq, 1.2, 3, 0, Size(60, 60), Size(120, 120));
				detector_ojosDer.detectMultiScale(im_ojos_gris, vect_ojosDer, 1.2, 3, 0, Size(60, 60), Size(120, 120));
				detector_ojos.detectMultiScale(im_ojos_equa, vect_ojosE, 1.2, 3, 0, Size(60, 60), Size(120, 120));
				detector_ojosIzq.detectMultiScale(im_ojos_equa, vect_ojosIzqE, 1.2, 3, 0, Size(60, 60), Size(120, 120));
				detector_ojosDer.detectMultiScale(im_ojos_equa, vect_ojosDerE, 1.2, 3, 0, Size(60, 60), Size(120, 120));

				if (vect_ojos.empty())
				{
					//	cout << "Oso binarizados2" << endl;
					detector_ojos.detectMultiScale(im_ojos_binE, vect_ojos, 1.2, 3, 0, Size(60, 60));
				}
				/*		if (vect_ojosIzq.empty())
				{
				cout << "Oso bilateral Izq" << endl;
				detector_ojosIzq.detectMultiScale(im_ojos_binE, vect_ojosIzq, 1.2, 3, 0, Size(60, 60));

				}
				if (vect_ojosDer.empty())
				{
				cout << "Oso bilateral Der" << endl;
				detector_ojosDer.detectMultiScale(im_ojos_binE, vect_ojosDer, 1.2, 3, 0, Size(60, 60));
				}*/

				for (Rect rectOjos : vect_ojos)
				{
					oj.push_back({ rectOjos.x + rectOjos.width / 2,  rectOjos.y + ho + rectOjos.height / 2, rectOjos.width, rectOjos.height });
				}
				for (Rect rectOjos : vect_ojosIzq)
				{
					oj.push_back({ rectOjos.x + rectOjos.width / 2,  rectOjos.y + ho + rectOjos.height / 2, rectOjos.width, rectOjos.height });
				}
				for (Rect rectOjos : vect_ojosDer)
				{
					//cout << "vect_ojosDer" << endl;
					oj.push_back({ rectOjos.x + rectOjos.width / 2,  rectOjos.y + ho + rectOjos.height / 2, rectOjos.width, rectOjos.height });
					//	circle(imagen, Point(rectOjos.x + rectOjos.width / 2, rectOjos.y+ h + rectOjos.height / 2), (sqrt((rectOjos.width / 2)*(rectOjos.width / 2) + (rectOjos.height / 2) ^ 2)), Scalar(100, 0, 0),1);
				}
				for (Rect rectOjos : vect_ojosE)
				{
					//cout << "vect_ojosE" << endl;
					oj.push_back({ rectOjos.x + rectOjos.width / 2,  rectOjos.y + ho + rectOjos.height / 2, rectOjos.width, rectOjos.height });
					//	rectangle(imagen, Point(rectOjos.x, rectOjos.y + h), Point(rectOjos.x + rectOjos.width, (rectOjos.y+ h) + rectOjos.height), Scalar(0, 255, 0), 3);
				}
				for (Rect rectOjos : vect_ojosIzqE)
				{
					//cout << "vect_ojosIzqE" << endl;
					oj.push_back({ rectOjos.x + rectOjos.width / 2,  rectOjos.y + ho + rectOjos.height / 2, rectOjos.width, rectOjos.height });
					//	rectangle(imagen, Point(rectOjos.x, rectOjos.y+ h), Point(rectOjos.x + rectOjos.width, rectOjos.y + h+ rectOjos.height), Scalar(0, 100, 0), 2);
				}
				for (Rect rectOjos : vect_ojosDerE)
				{
					//cout << "vect_ojosDerE" << endl;
					oj.push_back({ rectOjos.x + rectOjos.width / 2,  rectOjos.y + ho + rectOjos.height / 2, rectOjos.width, rectOjos.height });
					//	rectangle(imagen, Point(rectOjos.x, rectOjos.y+ h), Point(rectOjos.x + rectOjos.width, rectOjos.y +h+ rectOjos.height), Scalar(0, 0, 255), 1);
				}

				if (oj.empty()) {}
				else
				{
					cout << "Ojos: la size original es. " << oj.size() << endl;
					//	cout << oj[0].x << "  es la x inicial" << " y los param " << (oj[0].x + 12) << " " << (oj[0].x - 12) << endl;
					/*	for (int i = 1; i < oj.size(); i++)
					{
					cout << "x: " << oj[i].x << " y : " << oj[i].y << endl;
					if (oj[i].x< (oj[0].x + 12) && oj[i].x >(oj[0].x - 12))
					{
					cout << "borro el " << oj[i].x << endl;
					oj.erase(oj.begin() + i);
					i--;
					}
					}
					for (int i = 2; i < oj.size(); i++)
					{
					cout << "2.x: " << oj[i].x << " 2.y : " << oj[i].y << endl;
					if (oj[i].x< (oj[1].x + 12) && oj[i].x >(oj[1].x - 12))
					{
					cout << "2.borro el " << oj[i].x << endl;
					oj.erase(oj.begin() +i);
					i--;
					}
					}
					*/
					for (int j = 0; j < oj.size() - 1; j++)
					{
						//cout << oj[j].x << "  es la x inicial" << " y los param " << (oj[j].x + 12) << " " << (oj[j].x - 12) << endl;
						for (int i = j + 1; i < oj.size(); i++)
						{
							//cout << "Datos intermedios x: " << oj[i].x << " y : " << oj[i].y << endl;
							if (oj[i].x< (oj[j].x + 12) && oj[i].x >(oj[j].x - 12))
							{
								//cout << "2.borro el " << oj[i].x << endl;
								oj.erase(oj.begin() + i);
								i--;
							}
						}
					}
					//cout << "2.la nueva size es : " << oj.size() << endl;

					/*	for (int i = 0; i < oj.size(); i++)
					{
					cout << "Datos x: " << oj[i].x << " y : " << oj[i].y << endl;
					rectangle(imagen, Point(oj[i].x - (oj[i].ancho / 2), oj[i].y - (oj[i].alto / 2)), Point(oj[i].x + (oj[i].ancho / 2), oj[i].y + (oj[i].alto / 2)), Scalar(0, 0, 255), 1);
					}*/

				}

				if (oj.size() > 2)
				{
					//		cout << "Ojos: tengo mas de dos ojos" << endl;
				}
				for (int i = 0; i < oj.size(); i++)
				{
					//	cout << "Ojos: Datos x: " << oj[i].x << " y : " << oj[i].y << endl;
					rectangle(Rim_imagen, Point(oj[i].x - (oj[i].ancho / 2), oj[i].y - (oj[i].alto / 2)), Point(oj[i].x + (oj[i].ancho / 2), oj[i].y + (oj[i].alto / 2)), Scalar(0, 0, 255), 3);
					circle(Rim_imagen, Point(oj[i].x, oj[i].y), 10, Scalar(255, 0, 0), 2);
				}
			}





			if (detectBoca) {
				vector<Rect>  vect_boca, vect_boca1, vect_bocaE, vect_boca1E, vect_bocaG, vect_boca1G;										//Vector para guardar las caracteristicas de la boca
				im_boca_gris = Rim_gris(Rect(0, Rim_gris.size().height * 21 / 32, Rim_gris.size().width, Rim_gris.size().height * 5 / 16));	namedWindow("B1", WINDOW_AUTOSIZE);	moveWindow("B1", 500, 500); imshow("B1", im_boca_gris);
				//im_boca_equa = Rim_equa(Rect(0, Rim_equa.size().height * 21 / 32, Rim_equa.size().width, Rim_equa.size().height * 5 / 16));	namedWindow("B2", WINDOW_AUTOSIZE);	moveWindow("B2", 500, 500); imshow("B2", im_boca_equa);
				int hb = Rim_gris.size().height * 21 / 32;
				//cout << "Boca: Modify valor original :" << ImagenModify << endl;
				/*		if (im_boca_gris.size().width < 300) {
				cout << "Boca: imagen peke- Modify valor :" << ImagenModify << endl;
				if (ImagenModify != 2)
				{
				for (int i = 0; i < oj.size(); i++)
				{
				cout << "duplico ojos" << endl;
				oj[i].x = oj[i].x * 2;
				oj[i].y = oj[i].y * 2;
				oj[i].alto = oj[i].alto * 2;
				oj[i].ancho = oj[i].ancho * 2;
				}
				cout << "Boca: hago al imagen grande" << endl;
				ImagenModify = 2;
				pyrUp(imagen, imagen, Size(imagen.cols * 2, imagen.rows * 2));
				}
				pyrUp(im_boca_gris, im_boca_gris, Size(im_boca_gris.cols * 2, im_boca_gris.rows * 2));
				//pyrUp(im_boca_equa, im_boca_equa, Size(im_boca_equa.cols * 2, im_boca_equa.rows * 2));
				hb = hb * 2;
				}
				else if (ImagenModify == 2) {
				//cout << "imagen peque boca else" << endl;
				pyrUp(im_boca_gris, im_boca_gris, Size(im_boca_gris.cols * 2, im_boca_gris.rows * 2));
				hb = hb * 2;
				}
				*/
				//namedWindow("t", WINDOW_AUTOSIZE);	moveWindow("t", 0, 700); imshow("t", imagen);

				//cout << "Boca: tamanio de la imagen equa" << imagen.size() << endl;

				detector_boca.detectMultiScale(im_boca_gris, vect_bocaG, 1.2, 3, 0, Size(60, 60));
				//detector_boca.detectMultiScale(im_boca_equa, vect_bocaE, 1.2, 3, 0, Size(60, 60));
				//detector_boca1.detectMultiScale(im_boca_gris, vect_boca1G, 1.2, 3, 0, Size(60, 60));
				//detector_boca1.detectMultiScale(im_boca_equa, vect_boca1E, 1.2, 3, 0, Size(60, 60));
				for (Rect rectBoca : vect_bocaG)
				{
					//cout << "vect_bocaG" << endl;
					bo.push_back({ rectBoca.x + rectBoca.width / 2,  rectBoca.y + hb + rectBoca.height / 2, rectBoca.width, rectBoca.height });
					//rectangle(imagen, Point(rectBoca.x, rectBoca.y + hb), Point(rectBoca.x + rectBoca.width, rectBoca.y + hb + rectBoca.height), Scalar(255, 0, 0), 3);
					ellipse(Rim_imagen, Point(rectBoca.x + rectBoca.width / 2, rectBoca.y + hb + rectBoca.height / 2), Size(rectBoca.width / 2, rectBoca.height / 2), 0, 0, 360, Scalar(255, 0, 0), 1);
					//		cout << "Boca: Punto medio boca: " << (rectBoca.x + (rectBoca.width / 2)) << " y : " << (rectBoca.y + hb + (rectBoca.height / 2)) << endl;
					//		cout << "Boca: Anchura - " << rectBoca.width << " Altura - " << rectBoca.height << endl;
				}
				/*	for (Rect rectBoca : vect_bocaE)
				{
				cout << "vect_bocaE" << endl;
				rectangle(imagen, Point(rectBoca.x, rectBoca.y + h), Point(rectBoca.x + rectBoca.width, rectBoca.y + h + rectBoca.height), Scalar(0, 0, 100),3);
				}*/
				/*	for (Rect rectBoca : vect_boca1G)
				{
				cout << "vect_bocaG1" << endl;
				rectangle(imagen, Point(rectBoca.x, rectBoca.y + h), Point(rectBoca.x + rectBoca.width, rectBoca.y + h + rectBoca.height), Scalar(0,255, 0), 1);
				}*/
				/*	for (Rect rectBoca : vect_boca1E)
				{
				cout << "vect_bocaE1" << endl;
				rectangle(imagen, Point(rectBoca.x, rectBoca.y + h), Point(rectBoca.x + rectBoca.width, rectBoca.y + h + rectBoca.height), Scalar(0,100, 0), 1);
				}*/
				//	v_boca = vect_bocaG;
			}
			if (detectNariz) {
				vector<Rect> vect_nariz, vect_nariz1, vect_nariz2, vect_narizE, vect_nariz1E, vect_nariz2E;			//Vector para guardar las características de la nariz
				im_gris_nariz = Rim_gris(Rect(Rim_gris.size().width / 8, Rim_gris.size().height * 1 / 4, Rim_gris.size().width * 6 / 8, Rim_gris.size().height * 4 / 8));
				//Mat im_equa_nariz = Rim_equa(Rect(Rim_equa.size().width / 8, Rim_equa.size().height * 1 / 4, Rim_equa.size().width * 6 / 8, Rim_equa.size().height * 4 / 8));
				int hn = Rim_gris.size().height * 1 / 4;
				int wn = Rim_gris.size().width / 8;
				//cout << "Nariz: Modify valor original :" << ImagenModify << endl;
				if (im_gris_nariz.size().width < 250) {
					//cout << "Nariz: Imagen peke - Modify valor  :" << ImagenModify << endl;
					/*			if (ImagenModify != 2)
					{
					for (int i = 0; i < oj.size(); i++)
					{
					//	cout << "duplico ojos" << endl;
					oj[i].x = oj[i].x * 2;
					oj[i].y = oj[i].y * 2;
					oj[i].alto = oj[i].alto * 2;
					oj[i].ancho = oj[i].ancho * 2;
					}
					cout << "Nariz: hago la imagen grande" << endl;
					ImagenModify = 2;
					pyrUp(imagen, imagen, Size(imagen.cols * 2, imagen.rows * 2));
					}*/
					pyrUp(im_gris_nariz, im_gris_nariz, Size(im_gris_nariz.cols * 2, im_gris_nariz.rows * 2));
					//pyrUp(im_equa_nariz, im_equa_nariz, Size(im_equa_nariz.cols * 2, im_equa_nariz.rows * 2));
					hn = hn * 2;
					wn = wn * 2;
				}
				/*			else if (ImagenModify == 2) {
				//	cout << "imagen peque Nariz else" << endl;
				pyrUp(im_gris_nariz, im_gris_nariz, Size(im_gris_nariz.cols * 2, im_gris_nariz.rows * 2));
				//pyrUp(im_equa_nariz, im_equa_nariz, Size(im_equa_nariz.cols * 2, im_equa_nariz.rows * 2));
				hn = hn * 2;
				wn = wn * 2;
				}*/

				//cout << "Nariz : tamanio de la imagen equa" << imagen.size() << endl;
				namedWindow("Nariz", WINDOW_AUTOSIZE);	moveWindow("Nariz", 0, 0); imshow("Nariz", im_gris_nariz);

				//detector_nariz.detectMultiScale(im_gris_nariz, vect_nariz, 1.2, 3, 0, Size(60, 60));
				//detector_nariz2.detectMultiScale(im_gris_nariz, vect_nariz1, 1.2, 3, 0, Size(60, 60));
				detector_nariz3.detectMultiScale(im_gris_nariz, vect_nariz2, 1.2, 3, 0, Size(60, 60));
				//detector_nariz.detectMultiScale(im_equa_nariz, vect_narizE, 1.2, 3, 0, Size(60, 60));
				//detector_nariz2.detectMultiScale(im_equa_nariz, vect_nariz1E, 1.2, 3, 0, Size(60, 60));
				//detector_nariz3.detectMultiScale(im_equa_nariz, vect_nariz2E, 1.2, 3, 0, Size(60, 60));


				/*		for (Rect rectNariz : vect_nariz)
				{
				cout << "vect_nariz" << endl;
				rectangle(imagen, Point(rectNariz.x+w, rectNariz.y+h), Point(rectNariz.x +w+ rectNariz.width, (rectNariz.y+h) + rectNariz.height), Scalar(0, 255, 0), 3);
				}
				for (Rect rectNariz : vect_nariz1)
				{
				cout << "vect_nariz1" << endl;
				rectangle(imagen, Point(rectNariz.x+w, rectNariz.y+h), Point(rectNariz.x +w+ rectNariz.width, (rectNariz.y+h) + rectNariz.height), Scalar(0, 150, 0), 2);

				//rectangle(imagen, Point(rectNariz.x, rectNariz.y + h), Point(rectNariz.x + rectNariz.width, (rectNariz.y + h) + rectNariz.height), Scalar(0, 150, 0), 2);
				}*/
				for (Rect rectNariz : vect_nariz2)
				{
					circle(Rim_imagen, Point(rectNariz.x + wn + rectNariz.width / 2, rectNariz.y + hn + rectNariz.height / 2), (sqrt((rectNariz.width / 2)*(rectNariz.width / 2) + (rectNariz.height / 2) ^ 2)), Scalar(0, 255, 0), 1);
					circle(Rim_imagen, Point(rectNariz.x + wn + rectNariz.width / 2, rectNariz.y + hn + rectNariz.height / 2), 10, Scalar(255, 0, 0), 2);
					//cout << "vect_nariz2" << endl;
					na.push_back({ rectNariz.x + wn + rectNariz.width / 2,  rectNariz.y + hn + rectNariz.height / 2, rectNariz.width, rectNariz.height });
					cout << "Nariz: Punto medio nariz: " << (rectNariz.x + wn + rectNariz.width / 2) << " y : " << (rectNariz.y + hn + (rectNariz.height / 2)) << " Anchura : " << rectNariz.width << " Altura : " << rectNariz.height << endl;
					//rectangle(imagen, Point(rectNariz.x+w, rectNariz.y+h), Point(rectNariz.x +w+ rectNariz.width, (rectNariz.y+h) + rectNariz.height), Scalar(0, 0, 255), 1);
				}
				/*	for (Rect rectNariz : vect_narizE)
				{
				cout << "vect_narizE" << endl;
				circle(imagen, Point(rectNariz.x +w + rectNariz.width / 2, rectNariz.y +h+ rectNariz.height / 2), (sqrt((rectNariz.width / 2)*(rectNariz.width / 2) + (rectNariz.height / 2) ^ 2)), Scalar(255, 0, 0), 3);
				//circle(imagen, Point(rectNariz.x + rectNariz.width / 2, rectNariz.y + h + rectNariz.height / 2), (sqrt((rectNariz.width / 2)*(rectNariz.width / 2) + (rectNariz.height / 2) ^ 2)), Scalar(255, 0, 0), 3);
				}
				for (Rect rectNariz : vect_nariz1E)
				{
				cout << "vect_nariz1E" << endl;
				circle(imagen, Point(rectNariz.x+w + rectNariz.width / 2, rectNariz.y+h + rectNariz.height / 2), (sqrt((rectNariz.width / 2)*(rectNariz.width / 2) + (rectNariz.height / 2) ^ 2)), Scalar(150, 0, 0),2);
				}*/
				/*for (Rect rectNariz : vect_nariz2E)
				{
				cout << "vect_nariz2E" << endl;
				circle(imagen, Point(rectNariz.x +w+ rectNariz.width / 2, rectNariz.y +h + rectNariz.height / 2), (sqrt((rectNariz.width / 2)*(rectNariz.width / 2) + (rectNariz.height / 2) ^ 2)), Scalar(0, 255, 0), 1);
				}*/
				//		v_nariz = vect_nariz2;
			}


			/**************************************************************/
			/******************* REGLAS PARA IDENTIFCAR POSICION **********/
			/**************************************************************/
			if (Reglas) {
				cara Micara;

				/***************Numero de bocas********************/
				int boFlag = 1;
				if (bo.size() > 1) {
					//	cout << "Reglas: Tengo dos bocas" << endl;
					boFlag = 2;
				}
				else if (bo.size() == 0) {
					//	cout << "Reglas: No tengo boca" << endl;
					boFlag = 0;
				}
				/***************Numero de nariz********************/
				int naFlag = 1;
				if (na.size() > 1) {
					cout << "Reglas: Tengo dos narices" << endl;
					naFlag = 2;
				}
				else if (na.size() == 0) {
					cout << "Reglas: No tengo nariz" << endl;
					naFlag = 0;
				}
				if (naFlag == 1) Micara.Cnariz = na[0];
				/***************Identificar la boca********************/
				if (boFlag == 1)	Micara.Cboca = bo[0];
				else if (boFlag == 2) {
					//		cout << "Reglas: Para boca compruebo nariz" << endl; 
					if (naFlag == 1) {
						//			cout << "Reglas: Compruebo bocas" << endl;
						for (int j = 1; j < bo.size(); j++)
						{
							/*	if (bo[j].x < (na[0].x - na[0].ancho) || bo[j].x > (na[0].x + na[0].ancho))
							{
							cout << "Reglas: borro la boca " << bo[j].x << endl;
							bo.erase(bo.begin() + j);
							j--;
							}*/
							if (abs(bo[j].x - na[0].x) < abs(bo[j - 1].x - na[0].x))
							{
								//			cout << "Reglas: borro la boca " << bo[j-1].x << endl;
								bo.erase(bo.begin() + j - 1);
							}
							else
							{
								//			cout << "Reglas: borro la boca " << bo[j].x << endl;
								bo.erase(bo.begin() + j);
							}
							j--;
						}
					}
				}
				/*************Pintar las bocas resultantes*****/
				for (int i = 0; i < bo.size(); i++)
				{
					cout << "Reglas-Boca: Punto medio boca: " << bo[i].x << " y : " << bo[i].y << " Anchura : " << bo[i].ancho << " Altura : " << bo[i].alto << endl;
					rectangle(Rim_imagen, Point(bo[i].x - (bo[i].ancho / 2), bo[i].y - (bo[i].alto / 2)), Point(bo[i].x + (bo[i].ancho / 2), bo[i].y + (bo[i].alto / 2)), Scalar(0, 255, 0), 1);
					circle(Rim_imagen, Point(bo[i].x, bo[i].y), 10, Scalar(255, 0, 0), 3);
					if (i == 0) boFlag = 1;
					else boFlag = 2;
				}

				/************Determinar perfil***********************/
				if (naFlag == 1) {
					//	cout << "el naflag es : " << naFlag << "nariz " << na[0].x + 20 << "con " << imagen.size().width * 4 / 5 << endl;
					if (na[0].x + 12 > Rim_imagen.size().width * 3 / 5) {
						//	cout << "Perfil der" << endl;
						perfil = 2;
						putText(Rim_imagen, "Perfil der " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255), 6);
					}
					else if (na[0].x - 12 < Rim_imagen.size().width * 2 / 5) {
						//	cout << "Perfil izq" << endl;
						perfil = 1;
						putText(Rim_imagen, "Perfil izq " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255), 6);
					}
				}
				/*	else if (boFlag == 1) {
				if (bo[0].x + 12 > imagen.size().width * 3 / 5) {
				cout << "--- Perfil der" << endl;
				perfil = 2;
				putText(imagen, "Perfil der " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255), 6);
				}
				else if (bo[0].x - 12 < imagen.size().width * 2 / 5) {
				cout << "--- Perfil izq" << endl;
				perfil = 1;
				putText(imagen, "Perfil izq " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255), 6);
				}
				}*/

				/****************************Reglas para los ojos***********************************/
				int ojosDer = 0; int ojosIzq = 0; int ojFlag = 0;
				if (!oj.empty())
				{
					/*****Borro los no pueden ser ojos por geometría facial*****/
					for (int i = 0; i < oj.size(); i++)
					{
						if ((oj[i].x + 10 > Rim_imagen.size().width * 4 / 5) || (oj[i].x - 10 < Rim_imagen.size().width * 1 / 5))
						{
							//	cout << "--- Reglas - Ojos: borro los super alejados " << oj[i].x << endl;
							oj.erase(oj.begin() + i);
							i--;
						}
						else if (perfil == 0 && naFlag == 1)
						{
							if (oj[i].x > (Rim_imagen.size().width * 2 / 5) && oj[i].x <(Rim_imagen.size().width * 3 / 5))
								//if (oj[i].x < (na[0].x + na[0].ancho / 3) && oj[i].x >(na[0].x - na[0].ancho / 3))
							{
								//		cout << "--- Reglas - Ojos: Con cara frontal, borro los ojos del medio" << endl;
								oj.erase(oj.begin() + i);
								i--;
							}
						}
					}

					/**********Clasifico los restantes*********/
					for (int i = 0; i < oj.size(); i++)
					{
						if (naFlag == 1) {
							if (oj[i].x < na[0].x) {
								ojosIzq++;
								ojI.push_back({ oj[i].x,  oj[i].y , oj[i].ancho, oj[i].alto });
							}
							else if (oj[i].x > na[0].x) {
								ojosDer++;
								ojD.push_back({ oj[i].x,  oj[i].y , oj[i].ancho, oj[i].alto });
							}
						}
						/*	else if (boFlag == 1) {
						if (oj[i].x < bo[0].x) {
						ojosIzq++;
						ojI.push_back({ oj[i].x,  oj[i].y , oj[i].ancho, oj[i].alto, 0 });
						}
						else if (oj[i].x > bo[0].x) {
						ojosDer++;
						ojD.push_back({ oj[i].x,  oj[i].y , oj[i].ancho, oj[i].alto, 0 });
						}
						}*/
						else if (oj[i].x < Rim_imagen.size().width * 5 / 10) {
							ojosIzq++;
							ojI.push_back({ oj[i].x,  oj[i].y , oj[i].ancho, oj[i].alto });
						}
						else if (oj[i].x > Rim_imagen.size().width * 5 / 10) {
							ojosDer++;
							ojD.push_back({ oj[i].x,  oj[i].y , oj[i].ancho, oj[i].alto });
						}
					}

					cout << "Reglas-Clasificacion Ojos: Tengo drx: " << ojosDer << " e Izq: " << ojosIzq << endl;

					/**********Eliminar ojos no correctos***************/
					if (ojosIzq > 1)
					{
						int centroDx, centroDy;
						if (ojosDer > 1 && naFlag == 1)
						{
							int dist_derX = 0; int dist_derY = 0; int dDer = 0;
							for (int od = 0; od < ojD.size(); od++)
							{
								dist_derX = dist_derX + ojD[od].x;
								dist_derY = dist_derY + ojD[od].y;
							}
							dist_derX = dist_derX / ojD.size();
							dist_derY = dist_derY / ojD.size();
							//	circle(imagen, Point(dist_derX, dist_derY), 10, Scalar(0, 0, 255), 2);
							for (int od = 0; od < ojD.size(); od++)
							{
								if (dDer < sqrt(pow(abs(ojD[od].x - dist_derX), 2) + (pow(abs(ojD[od].y - dist_derY), 2))))
									dDer = sqrt(pow(abs(ojD[od].x - dist_derX), 2) + (pow(abs(ojD[od].y - dist_derY), 2)));
							}
							//	cout << dDer << "es la distancia der " << endl;

							int dist_izqX = 0; int dist_izqY = 0; int dIzq = 0;
							for (int oi = 0; oi < ojI.size(); oi++)
							{
								dist_izqX = dist_izqX + ojI[oi].x;
								dist_izqY = dist_izqY + ojI[oi].y;
							}
							dist_izqX = dist_izqX / ojI.size();
							dist_izqY = dist_izqY / ojI.size();
							//	circle(imagen, Point(dist_izqX, dist_izqY), 10, Scalar(0, 0, 255), 2);
							for (int oi = 0; oi < ojI.size(); oi++)
							{
								if (dIzq < sqrt(pow(abs(ojI[oi].x - dist_izqX), 2) + (pow(abs(ojI[oi].y - dist_izqY), 2))))
									dIzq = sqrt(pow(abs(ojI[oi].x - dist_izqX), 2) + (pow(abs(ojI[oi].y - dist_izqY), 2)));
							}
							//	cout << dIzq << "es la distancia izq " << endl;
							if (dIzq > dDer) {
								int distanciaDn = pow(abs(dist_derY - na[0].y), 2) + pow(abs(dist_derX - na[0].x), 2);
								int distanciaI0n = pow(abs(ojI[0].y - na[0].y), 2) + pow(abs(ojI[0].x - na[0].x), 2);
								int DiferenciaDist = abs(distanciaDn - distanciaI0n);
								for (int j = 1; j < ojI.size(); j++) {
									int distanciaIjn = pow(abs(ojI[j].y - na[0].y), 2) + pow(abs(ojI[j].x - na[0].x), 2);
									if (abs(distanciaIjn - distanciaDn) >= DiferenciaDist) {
										ojI.erase(ojI.begin() + j);
										j--;
									}
									else
									{
										ojI.erase(ojI.begin());
										DiferenciaDist = abs(distanciaIjn - distanciaDn);
										j--;
									}
									ojosIzq--;
								}
							}
							if (dIzq < dDer) {
								int distanciaIn = pow(abs(dist_izqY - na[0].y), 2) + pow(abs(dist_izqX - na[0].x), 2);
								int distanciaD0n = pow(abs(ojD[0].y - na[0].y), 2) + pow(abs(ojD[0].x - na[0].x), 2);
								int DiferenciaDist = abs(distanciaIn - distanciaD0n);
								//	cout << "---- Reglas: Ojos : comparo ojos distancias con nariz" << endl;
								for (int j = 1; j < ojD.size(); j++) {
									int distanciaDjn = pow(abs(ojD[j].y - na[0].y), 2) + pow(abs(ojD[j].x - na[0].x), 2);
									if (abs(distanciaDjn - distanciaIn) > DiferenciaDist) {
										//		cout << "Reglas Ojos: Borro en la compracion de distancias " << endl;
										ojD.erase(ojD.begin() + j);
										j--;
									}
									else
									{
										//	cout << "--- Reglas Ojos: Distancia: adjudico al ojoI-0 el siguiente" << endl;
										//	ojD[0] = ojD[j];
										//	ojD.erase(ojD.begin() + j
										ojD.erase(ojD.begin());
										DiferenciaDist = abs(distanciaDjn - distanciaIn);
										j--;
									}
									ojosDer--;
								}
							}
						}
						//	cout << "--- Reglas Ojos: tengo ojos izquierdos : " << ojosIzq << endl;
						/*			if (naFlag == 1) {
						for (int j = 0; j < ojI.size(); j++)
						{
						//cout << "Reglas: Datos iniciales x: " << oj[j].x << " y : " << oj[j].y << endl;
						if (ojI[j].x< (na[0].x + na[0].ancho / 8) && ojI[j].x>(na[0].x - na[0].ancho / 8))
						{
						cout << "--NO DEBERIA- Reglas: Ojos borro el central izq " << ojI[j].x << endl;
						ojI.erase(ojI.begin() + j);
						j--;
						ojosIzq--;
						}
						//else if (abs(ojI[j].x - (na[0].x )) > imagen.size().width * 1.5f / 5)
						/*		else if (ojI[j].x <  imagen.size().width * 1 / 5)
						{
						cout << "Reglas: Ojos borro izq super alejado " << ojI[j].x << endl;
						ojI.erase(ojI.begin() + j);
						j--;
						ojosIzq--;
						}*/
						//			}
						//		}
						if (naFlag == 1 && ojosDer == 1 && perfil == 0) {
							int distanciaDn = pow(abs(ojD[0].y - na[0].y), 2) + pow(abs(ojD[0].x - na[0].x), 2);
							int distanciaI0n = pow(abs(ojI[0].y - na[0].y), 2) + pow(abs(ojI[0].x - na[0].x), 2);
							int DiferenciaDist = abs(distanciaDn - distanciaI0n);
							/*		cout << "Ojos D: " << ojD[0].x << " y el y " << ojD[0].y << endl;
							cout << "Ojos 0: " << ojI[0].x << " y el y " << ojI[0].y << endl;
							cout << "Nariz: " << na[0].x << " y el y " << na[0].y << endl;
							cout << "---- Reglas - Ojos: La distancia del derecho a la nariz es " << distanciaDn << endl;
							cout << "---- Reglas - Ojos: La distancia del izq0  la nariz es : " << distanciaI0n << endl;
							cout << "---- Reglas: Ojos : comparo ojos distancias con nariz" << endl;*/
							for (int j = 1; j < ojI.size(); j++) {
								//	cout << "Ojos 1: " << ojI[j].x << " y el y " << ojI[j].y << endl;
								int distanciaIjn = pow(abs(ojI[j].y - na[0].y), 2) + pow(abs(ojI[j].x - na[0].x), 2);
								//	cout << "--- Reglas Ojos: La nueva distancia izq " << distanciaIjn << endl;
								if (abs(distanciaIjn - distanciaDn) >= DiferenciaDist) {

									//			cout << "--- Reglas Ojos: Borro el nuevo en comparacion de distancias " << endl;
									ojI.erase(ojI.begin() + j);
									j--;
								}
								else
								{
									//		cout << "--- Reglas Ojos: Distancia: adudico al ojoI-0 el sigueinte" << endl;
									//				ojI[0] = ojI[j];
									//				ojI.erase(ojI.begin() + j
									ojI.erase(ojI.begin());
									DiferenciaDist = abs(distanciaIjn - distanciaDn);
									j--;
								}
								ojosIzq--;
							}
						}
						else if (ojosDer == 1) {
							//	cout << "--Reglas: Ojos : comparo ojos Izqs con uno derecho en y.Entro si no tengo nariz" << endl;
							for (int i = 1; i < ojI.size(); i++) {
								if (abs(ojI[i].y - ojD[0].y) > abs(ojI[0].y - ojD[0].y)) {
									//		cout << "Reglas Ojos: Borro en la compracion de y " << endl;
									ojI.erase(ojI.begin() + i);
									i--;
								}
								else
								{
									//		cout << "--- Reglas Ojos: adudico al ojoI-0 el sigueinte" << endl;
									ojI[0] = ojI[i];
									ojI.erase(ojI.begin() + i);
									i--;
								}
								ojosIzq--;
							}
						}
					}

					if (ojosDer > 1)
					{
						//	cout << "-- Reglas Ojos: compruebo Der " << endl;
						if (ojosDer > 1)
						{
							//		cout << " -- Reglas Ojos: tengo ojos derechos : " << ojosDer << endl;
							if (naFlag == 1 && ojosDer > 1 && perfil == 0) {
								for (int j = 0; j < ojD.size(); j++)
								{
									//cout << "Reglas: Datos iniciales x: " << oj[j].x << " y : " << oj[j].y << endl;
									/**Borro el punto central de las gafas**/
									/*			if (ojD[j].x< (na[0].x + na[0].ancho / 8) && ojD[j].x>(na[0].x - na[0].ancho / 8))
									{
									cout << "--NO DEBERIA-- Reglas: Ojos borro el central der " << ojD[j].x << endl;
									ojD.erase(ojD.begin() + j);
									j--;
									ojosDer--;
									}*/
									//else if (abs(ojD[j].x - (na[0].x)) > imagen.size().width * 1.5f / 5)
									/*		else if (ojD[j].x >  imagen.size().width * 4 / 5)
									{
									cout << "Reglas: Ojos borro der super alejado " << ojD[j].x << endl;
									ojD.erase(ojD.begin() + j);
									j--;
									ojosDer--;
									}*/
								}
							}
							if (naFlag == 1 && ojosIzq == 1 && perfil == 0) {
								int distanciaIn = pow(abs(ojI[0].y - na[0].y), 2) + pow(abs(ojI[0].x - na[0].x), 2);
								int distanciaD0n = pow(abs(ojD[0].y - na[0].y), 2) + pow(abs(ojD[0].x - na[0].x), 2);
								int DiferenciaDist = abs(distanciaIn - distanciaD0n);
								//	cout << "---- Reglas: Ojos : comparo ojos distancias con nariz" << endl;
								for (int j = 1; j < ojD.size(); j++) {
									int distanciaDjn = pow(abs(ojD[j].y - na[0].y), 2) + pow(abs(ojD[j].x - na[0].x), 2);
									if (abs(distanciaDjn - distanciaIn) > DiferenciaDist) {
										//	cout << "Reglas Ojos: Borro en la compracion de distancias " << endl;
										ojD.erase(ojD.begin() + j);
										j--;
									}
									else
									{
										//	cout << "--- Reglas Ojos: Distancia: adjudico al ojoI-0 el siguiente" << endl;
										//	ojD[0] = ojD[j];
										//	ojD.erase(ojD.begin() + j
										ojD.erase(ojD.begin());
										DiferenciaDist = abs(distanciaDjn - distanciaIn);
										j--;
									}
									ojosDer--;
								}
							}
							else if (ojosIzq == 1) {
								cout << "--NO DEBERIA-- Reglas: Ojos : comparo ojos Izqs con uno derecho en y " << endl;
								for (int i = 1; i < ojD.size(); i++) {
									if (abs(ojD[i].y - ojI[0].y) > abs(ojD[0].y - ojI[0].y)) {
										ojD.erase(ojD.begin() + i);
										i--;
									}
									else
									{
										ojD[0] = ojD[i];
										ojD.erase(ojD.begin() + i);
										i--;
									}
									ojosDer--;
								}
							}
						}
					}
					/*			for (int j = 0; j < ojI.size(); j++)
					{
					//cout << "Reglas: Datos iniciales x: " << oj[j].x << " y : " << oj[j].y << endl;
					/**Borro el punto central de las gafas**/
					/*				if (oj[j].x< (na[0].x + na[0].ancho / 8) && oj[j].x>(na[0].x - na[0].ancho / 8))
					{
					cout << "Reglas: borro el " << oj[j].x << endl;
					oj.erase(oj.begin() + j);
					j--;
					}5
					}*/


					/****Identificar perfiles******/
					if (ojosIzq == 1 && ojosDer == 1 && perfil == 0) {
						//	cout << "Reglas : voy a identificar perfiles" << endl;
						cout << "diferencia de alturas " << abs(ojI[0].y - ojD[0].y) << endl;
						if (naFlag == 1) {
							//	cout << "Reglas : voy a identificar perfiles con nariz" << endl;
							if (((ojI[0].y < ojD[0].y + 40) && (ojI[0].y > ojD[0].y - 40)) || ((ojD[0].y < ojI[0].y + 40) && (ojD[0].y > ojI[0].y - 40))) {
								//		cout << "reglas Ojos : 2 alineados" << endl;
								int distanciaI = pow(abs(ojI[0].x - na[0].x), 2) + pow(abs(ojI[0].y - na[0].y), 2);//abs(ojI[0].x - na[0].x);
								int distanciaD = pow(abs(ojD[0].x - na[0].x), 2) + pow(abs(ojD[0].y - na[0].y), 2);//abs(ojD[0].x - na[0].x);
																												   //int DiferenciaDist = abs(distanciaI - distanciaD);
								if (distanciaD >(distanciaI*1.7f)) {// ((distanciaI < imagen.size().width / 10) || distanciaD >(distanciaI*1.5f)) {
																	//	cout << "Reglas Perfil: ahora si es el perfil izq" << endl;
									perfil = 1;
									putText(Rim_imagen, "Perfil izq " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(0, 255, 0), 6);
								}
								if (distanciaI >(distanciaD*1.7f)) {//((distanciaD < imagen.size().width / 10) || distanciaI >(distanciaD*1.5f)) {
																	//	cout << "Reglas Perfil: ahora si es el perfil der" << endl;
									perfil = 2;
									putText(Rim_imagen, "Perfil der " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(0, 255, 0), 6);

								}
							}
						}
						else
						{
							//	cout << "Reglas : voy a identificar perfiles sin nariz" << endl;
							//	cout << "ojI.y " << ojI[0].y << " ojD.y:" << ojD[0].y << endl;
							if (((ojI[0].y < ojD[0].y + 40) && (ojI[0].y > ojD[0].y - 40)) || ((ojD[0].y < ojI[0].y + 40) && (ojD[0].y > ojI[0].y - 40))) {
								//		cout << "reglas Ojos2 : 2 alineados" << endl;
								//int DiferenciaDist = abs(distanciaI - distanciaD);
								int distanciaI, distanciaD;
								if (boFlag == 1) {
									distanciaI = abs(ojI[0].x - bo[0].x);
									distanciaD = abs(ojD[0].x - bo[0].x);
								}
								else {
									distanciaI = abs(ojI[0].x - Rim_imagen.size().width * 5 / 10);
									distanciaD = abs(ojD[0].x - Rim_imagen.size().width * 5 / 10);
								}
								if ((distanciaI < Rim_imagen.size().width / 10) || distanciaD >(distanciaI*1.7f)) {
									//	cout << "Reglas Perfil: ahora si es el perfil izq" << endl;
									perfil = 1;
									putText(Rim_imagen, "Perfil izq " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(0, 255, 0), 6);
								}
								if ((distanciaD < Rim_imagen.size().width / 10) || distanciaI >(distanciaD*1.7f)) {
									//	cout << "Reglas Perfil: ahora si es el perfil der" << endl;
									perfil = 2;
									putText(Rim_imagen, "Perfil der " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(0, 255, 0), 6);

								}
							}
						}
					}
					else if (ojosIzq == 1 && perfil == 0) {
						if (naFlag == 1) {
							//	cout << "Reglas : voy a identificar 1OJO perfiles con nariz" << endl;
							int distanciaI = abs(ojI[0].x - na[0].x);
							if (distanciaI <(Rim_imagen.size().width * 1 / 20)) {// ((distanciaI < imagen.size().width / 10) || distanciaD >(distanciaI*1.5f)) {
								cout << "Reglas Perfil: ahora si es el perfil izq" << endl;
								perfil = 1;
								putText(Rim_imagen, "Perfil izq " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(0, 255, 0), 6);
							}
						}
						else
						{
							//	cout << "Reglas : voy a identificar 1OJO  perfiles sin nariz" << endl;
							int distanciaI = abs(ojI[0].x - Rim_imagen.size().width * 5 / 10);
							if (distanciaI <(Rim_imagen.size().width * 1 / 20)) {// ((distanciaI < imagen.size().width / 10) || distanciaD >(distanciaI*1.5f)) {
								cout << "Reglas Perfil: ahora si es el perfil izq" << endl;
								perfil = 1;
								putText(Rim_imagen, "Perfil izq " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(0, 255, 0), 6);
							}
						}
					}
					else if (ojosDer == 1 && perfil == 0) {
						if (naFlag == 1) {
							//	cout << "Reglas : voy a identificar 1OJO perfiles con nariz" << endl;
							int distanciaD = abs(ojD[0].x - na[0].x);
							if (distanciaD <(Rim_imagen.size().width * 1 / 10)) {	// ((distanciaI < imagen.size().width / 10) || distanciaD >(distanciaI*1.5f)) {
								cout << "Reglas Perfil: ahora si es el perfil der" << endl;
								perfil = 2;
								putText(Rim_imagen, "Perfil der " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(255, 0, 0), 6);
							}
						}
						else
						{
							//	cout << "Reglas : voy a identificar 1OJO  perfiles sin nariz" << endl;
							int distanciaD = abs(ojD[0].x - Rim_imagen.size().width * 5 / 10);
							if (distanciaD <(Rim_imagen.size().width * 1 / 20)) {// ((distanciaI < imagen.size().width / 10) || distanciaD >(distanciaI*1.5f)) {
								cout << "Reglas Perfil: ahora si es el perfil der" << endl;
								perfil = 2;
								putText(Rim_imagen, "Perfil der " + std::to_string(perfil), Point(40, 40), FONT_HERSHEY_PLAIN, 3, Scalar(255, 0, 0), 6);
							}
						}
					}
					if (ojosDer == 1 && ojosIzq == 1) {
						float anguloOjos = atan(float(ojI[0].y - ojD[0].y) / float(ojI[0].x - ojD[0].x));
						dist_ojos = sqrt(pow((ojI[0].y - ojD[0].y), 2) + pow((ojI[0].x - ojD[0].x), 2));
						cout << "distancias " << (float(ojI[0].y - ojD[0].y) / float(ojI[0].x - ojD[0].x)) << " " << atan(float(ojI[0].y - ojD[0].y) / float(ojI[0].x - ojD[0].x)) << "y " << anguloOjos*3.1415
							/ 180 << endl;
						if (alfa != -70) alfa = alfa + anguloOjos;
						else alfa = anguloOjos;
						cout << "El rostro está girado un ángulo : " << anguloOjos << " tiene perfil : " << perfil << " y la distancia entre ojos " << dist_ojos / factor << endl;

					}

					/******Dibujar ojos******************/
					for (int i = 0; i < ojI.size(); i++)
					{
						cout << "Reglas: Datos Ojos x: " << ojI[i].x << " y : " << ojI[i].y << endl;
						rectangle(Rim_imagen, Point(ojI[i].x - (ojI[i].ancho / 2), ojI[i].y - (ojI[i].alto / 2)), Point(ojI[i].x + (ojI[i].ancho / 2), ojI[i].y + (ojI[i].alto / 2)), Scalar(0, 255, 0), 1);
					}
					for (int i = 0; i < ojD.size(); i++)
					{
						cout << "Reglas: Datos Ojos x: " << ojD[i].x << " y : " << ojD[i].y << endl;
						rectangle(Rim_imagen, Point(ojD[i].x - (ojD[i].ancho / 2), ojD[i].y - (ojD[i].alto / 2)), Point(ojD[i].x + (ojD[i].ancho / 2), ojD[i].y + (ojD[i].alto / 2)), Scalar(0, 255, 0), 1);
					}
				}
			}

		}


		line(Rim_imagen, Point(Rim_imagen.size().width / 5, 0), Point(Rim_imagen.size().width / 5, Rim_imagen.size().height), Scalar(255, 0, 0), 1);
		line(Rim_imagen, Point(Rim_imagen.size().width * 2 / 5, 0), Point(Rim_imagen.size().width * 2 / 5, Rim_imagen.size().height), Scalar(255, 0, 0), 1);
		line(Rim_imagen, Point(Rim_imagen.size().width * 3 / 5, 0), Point(Rim_imagen.size().width * 3 / 5, Rim_imagen.size().height), Scalar(255, 0, 0), 1);
		line(Rim_imagen, Point(Rim_imagen.size().width * 4 / 5, 0), Point(Rim_imagen.size().width * 4 / 5, Rim_imagen.size().height), Scalar(255, 0, 0), 1);
		namedWindow("Rostro ", WINDOW_AUTOSIZE);	moveWindow("Rostro ", 900, 0); imshow("Rostro ", Rim_imagen);


		//Vídeo original con el rectángulo sobre el rostro detectado
		if (vect_cara.empty()) {}
		else
		{
			imshow("Deteccion de rostros", imagen);
			//	system("PAUSE"); //Espera entrada de usuario por teclado	
		}

		//Al presionar la tecla "x" ó "X" cerramos la cámara y salimos del programa
		if (waitKey(50) == 88 || waitKey(50) == 120) break;
	}
}