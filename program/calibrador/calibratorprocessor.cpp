 #include "calibratorprocessor.h"
#include "variaveisglobais.h"
#include <iostream>
#include <cmath>
#include <cstring>

using namespace std;

#define RAIO_SELECIONADO 5.0
// #define NUM_BUFFERS 4
// //#define NUM_BUFFERS 10
// #define ISO_SPEED DC1394_ISO_SPEED_400
// #define FRAME_RATE DC1394_FRAMERATE_30
// //#define DEBAYER_METHOD DC1394_BAYER_METHOD_NEAREST
// #define DEBAYER_METHOD DC1394_BAYER_METHOD_BILINEAR
// #define VIDEO_MODE DC1394_VIDEO_MODE_640x480_MONO8


 

CalibratorProcessor::CalibratorProcessor() :
  // ImBruta(0,0),
  //	ImBruta("imagem_clara.ppm"),
  Camera (CAM_FUTROBOT),
  ImProcessada(0,0),
  modo(CALIBRATOR_IMAGEM_REAL),
  //capturando(false),
  // novocamparam(false),
  //nPontosNotaveis(0),
  nRetas(0),
  LarguraCaptura(0), 
  AlturaCaptura(0),
  //nCores(0),
  //pixelsNotaveis(NULL),
  pontosImagemIniciais(NULL),
  //pontosNotaveis(NULL),
  retas(NULL),
  //limHPG(NULL),
  nomeCor(NULL),
  cores(NULL),
  coresInversas(NULL),
  //limiarPinf(PG_MIN_VALUE),
  //limiarPsup(PG_MAX_VALUE),
  corAtual(0),
  offset_u(0),
  offset_v(0),
  true_color(false)
{
  //nada
  
}

CalibratorProcessor::CalibratorProcessor(const char* arquivo) :
  //  ImBruta(0,0),
  //	ImBruta("imagem_clara.ppm"),
  Camera (CAM_FUTROBOT),	
  ImProcessada(0,0),
  modo(CALIBRATOR_IMAGEM_REAL),
  //capturando(false),
  //novocamparam(false),
  //nPontosNotaveis(0),
  nRetas(0),
  LarguraCaptura(0), 
  AlturaCaptura(0),
  //nCores(0),
  //pixelsNotaveis(NULL),
  pontosImagemIniciais(NULL),
  //pontosNotaveis(NULL),
  retas(NULL),
  //limHPG(NULL),
  nomeCor(NULL),
  cores(NULL),
  coresInversas(NULL),
  //limiarPinf(PG_MIN_VALUE),
  //limiarPsup(PG_MAX_VALUE),
  corAtual(0),
  offset_u(0),
  offset_v(0),
  true_color(false)
{
  if(readFile(arquivo)){
    exit(1);
  }
}

CalibratorProcessor::~CalibratorProcessor(){
  //    for(unsigned i=0; i < nCores; i++){
  //	delete cores[i].nome;
  //    }
  terminar(); // e' da Camera
  delete[] cores;
  delete[] coresInversas;
  //    delete[] limHPG;
  delete[] retas;
  //delete[] pontosNotaveis;
  delete[] pontosImagemIniciais;
  //delete[] pixelsNotaveis;
}

bool CalibratorProcessor::readFile(const char* arquivo)
{
  FILE *f;
  if(strlen(arquivo) == 0){
    f = fopen("../../etc/calibradorConfig.conf","r");
    //f = fopen("calibradorConfig.conf","r");
  }else{
    f = fopen(arquivo,"r");
  }
  if(f == NULL) return true;
  int true_color_local;
  if(fscanf(f,"TAMANHO DA IMAGEM:\n(Width Height TrueColor?)\n%d %d %d\n\n",
	    &LarguraCaptura,&AlturaCaptura,&true_color_local) != 3){
    fclose(f);
    return true;
  }
  true_color = (bool)true_color_local;
  ImBruta = ImagemRGB(LarguraCaptura,AlturaCaptura);
  ImProcessada = ImagemRGB(LARGURA_EXIBICAO,ALTURA_EXIBICAO);
    
  if(fscanf(f,"NUMERO DE PONTOS: %d\n(Xw Yw Xi Yi)\n",&calibracaoParam.nPontosNotaveis) != 1){
    fclose(f);
    return true;
  }
  calibracaoParam.pontosImagem = new Coord2[calibracaoParam.nPontosNotaveis];
  pontosImagemIniciais  = new Coord2[calibracaoParam.nPontosNotaveis];
  calibracaoParam.pontosReais = new Coord2[calibracaoParam.nPontosNotaveis];
  
  for(unsigned i=0;i<calibracaoParam.nPontosNotaveis;i++){
    if(fscanf(f,"%lf %lf %lf %lf\n",
	      &calibracaoParam.pontosReais[i].x(),
	      &calibracaoParam.pontosReais[i].y(),
	      &calibracaoParam.pontosImagem[i].u(),
	      &calibracaoParam.pontosImagem[i].v()) != 4){
      fclose(f);
      return true;
    }
    pontosImagemIniciais[i] = calibracaoParam.pontosImagem[i];
  }
    
  if(fscanf(f,"\nNUMERO DE RETAS: %d\n(p1 p2)\n",&nRetas) != 1){
    fclose(f);
    return true;
  }
    
  retas = new RETA[nRetas];
    
  for(unsigned i=0;i<nRetas;i++){
    if(fscanf(f,"%d %d\n",&retas[i].p1,&retas[i].p2) != 2){
      fclose(f);
      return true;
    }
  }
    
  if(fscanf(f,"\nNUMERO DE CORES: %d\n(strLen Nome R G B)\n",&calibracaoParam.nCores) != 1){
    fclose(f);
    return true;
  }
    
  calibracaoParam.limHPG = new limitesHPG[calibracaoParam.nCores];
  nomeCor = new char*[calibracaoParam.nCores];
  cores = new PxRGB[calibracaoParam.nCores];
  coresInversas = new PxRGB[calibracaoParam.nCores];
    
  int str_len;
  unsigned R,G,B;
  for(unsigned i=0; i < calibracaoParam.nCores; i++){
    if(fscanf(f,"%d ",&str_len) != 1){
      fclose(f);
      return true;
    }
    nomeCor[i] = new char[str_len+1];
    if(fscanf(f,"%s %u %u %u\n",
	      nomeCor[i],
	      &R, &G, &B) != 4){
      fclose(f);
      return true;
    }
    cores[i].r = (uint8_t)R;
    cores[i].g = (uint8_t)G;
    cores[i].b= (uint8_t)B;
    coresInversas[i].r = 255 - cores[i].r;
    coresInversas[i].g = 255 - cores[i].g;
    coresInversas[i].b = 255 - cores[i].b;

  }
    
  fclose(f);
    
  resetHPG();
  resetCameraParam();
    
  return false;
}

void CalibratorProcessor::setParameters(){
  ajusteparam(cameraParam);
  
}

bool CalibratorProcessor::saveCameraParam(const char* arquivo){
  return cameraParam.write(arquivo);
}

bool CalibratorProcessor::loadCameraParam(const char* arquivo){
  if(cameraParam.read(arquivo)) return true;
  setParameters();
    
  return false;
}



bool CalibratorProcessor::fileOpen(const char* text)
{
  return calibracaoParam.read(text);
}


bool CalibratorProcessor::fileSave(const char* arquivo)
{
  return calibracaoParam.write(arquivo);
}

void CalibratorProcessor::resetPixelsNotaveis(){
  for(unsigned i = 0; i < calibracaoParam.nPontosNotaveis; i++){
    calibracaoParam.pontosImagem[i] = pontosImagemIniciais[i];
  }  
}

void CalibratorProcessor::resetHPG(){
  calibracaoParam.limiarPInf = PG_MIN_VALUE;
  calibracaoParam.limiarPSup = PG_MAX_VALUE;
  for(unsigned i = 0; i < calibracaoParam.nCores; i++){	
    calibracaoParam.limHPG[i].H.min = H_MIN_VALUE;
    calibracaoParam.limHPG[i].H.max = H_MAX_VALUE;
    calibracaoParam.limHPG[i].P.min = PG_MIN_VALUE;
    calibracaoParam.limHPG[i].P.max =  PG_MAX_VALUE;
    calibracaoParam.limHPG[i].G.min = PG_MIN_VALUE;
    calibracaoParam.limHPG[i].G.max =  PG_MAX_VALUE;
  }
}

void CalibratorProcessor::resetCameraParam(){
  cameraParam.brightness = 0;
  cameraParam.exposure = 33;
  cameraParam.hue = 0;
  cameraParam.saturation = 62;
  cameraParam.gamma = 0;
  cameraParam.shutter = 0;
  cameraParam.gain = 0;
  
   setParameters ();
  
}

bool CalibratorProcessor::loadImage(const char* arq){
  return ImBruta.load(arq);
}

void CalibratorProcessor::saveImage(const char* arq){
  ImProcessada.save(arq);
}


bool CalibratorProcessor::processImage(){
  unsigned int i,j;
  int count=0, cor_pixel=0;
  float H, P, G;
     
  //ponteiros para percorrer as imagensRGB
  //    PxRGB *ptBruta;
  //    PxRGB *ptProcessada;
    
  //Cores padroes a serem usadas no processamento
  static PxRGB PxPreto(0,0,0);
  static PxRGB PxVermelho(255,0,0);
  static PxRGB PxCinza(127,127,127);
    
  switch(modo){
  case CALIBRATOR_IMAGEM_REAL:
    for(i = 0; i < ImProcessada.nlin(); i++){
      for(j = 0; j < ImProcessada.ncol(); j++){
	ImProcessada[i][j] = ImBruta[offset_v + i][offset_u + j];
      }
    }
    //	ImProcessada = ImBruta;
    break;
  case CALIBRATOR_LIMITES_P_E_PONTOS:
  case CALIBRATOR_LIMITES_P:
    /*
    //gera a imagem processada com os limites min e max de P
    ptBruta = (PxRGB*)ImBruta.getRawData();
    ptProcessada = (PxRGB*)ImProcessada.getRawData();
    //	unsigned i, j2, base1, base2;
    for(i=0;i<ImBruta.nlin()*ImBruta.ncol();i++){
    ptBruta->getHPG(H,P,G);
    ptProcessada->setHPG(H,P,G);
    ptBruta++;
    ptProcessada++;
    }
    */
    for(i = 0; i < ImProcessada.nlin(); i++){
      for(j = 0; j < ImProcessada.ncol(); j++){
	ImBruta[offset_v + i][offset_u + j].getHPG(H,P,G);
	ImProcessada[i][j].setHPG(H,P,G);
		
      }
    }
    if(modo == CALIBRATOR_LIMITES_P){
      break;
    }
  case CALIBRATOR_PONTOS:
    //desenha os pontos
    if(modo == CALIBRATOR_PONTOS){
      //	    ImProcessada = ImBruta;	    
      for(i = 0; i < ImProcessada.nlin(); i++){
	for(j = 0; j < ImProcessada.ncol(); j++){
	  ImProcessada[i][j] = ImBruta[offset_v + i][offset_u + j];
	}
      }
    }
    int jj,kk;
    for(i = 0; i < calibracaoParam.nPontosNotaveis; i++){
      for(jj = -2; jj <=2; jj++){
	for(kk = -2; kk <=2; kk++){
	  if((calibracaoParam.pontosImagem[i].u() + jj - offset_u) >= 0 &&
	     (calibracaoParam.pontosImagem[i].u() + jj - offset_u) < ImProcessada.ncol() &&
	     (calibracaoParam.pontosImagem[i].v() + kk - offset_v) >= 0 &&
	     (calibracaoParam.pontosImagem[i].v() + kk - offset_v) < ImProcessada.nlin()){
	    ImProcessada[(int)round(calibracaoParam.pontosImagem[i].v() + kk-offset_v)][(int)round(calibracaoParam.pontosImagem[i].u() + jj - offset_u)] = PxVermelho;
	  }
	}
      }
    }
	
    //desenha as retas
    double dU,dV,dM;
    unsigned u,v,j;
    for(i=0;i<nRetas;i++){
      dU = calibracaoParam.pontosImagem[retas[i].p2].u() - 
	calibracaoParam.pontosImagem[retas[i].p1].u();
      dV = calibracaoParam.pontosImagem[retas[i].p2].v() - 
	calibracaoParam.pontosImagem[retas[i].p1].v();
      dM = round(max(fabs(dU),fabs(dV)));
      for(j=0;j<(unsigned)dM;j++){
	u = (unsigned)round(calibracaoParam.pontosImagem[retas[i].p1].u() + (dU/dM)*j) -offset_u;
	v = (unsigned)round(calibracaoParam.pontosImagem[retas[i].p1].v() + (dV/dM)*j) -offset_v;
	if(u < ImProcessada.ncol()  &&
	   v < ImProcessada.nlin()) {
	  ImProcessada[v][u] = PxVermelho;
	}
      }
    }
    break;
  case CALIBRATOR_COR_ETIQUETADA:
  case CALIBRATOR_COR_ETIQUETADA_SOFT:
  case CALIBRATOR_IMAGEM_ERROS:
    for(i = 0; i < ImProcessada.nlin(); i++){
      for(j = 0; j < ImProcessada.ncol(); j++){
	count = 0;
	if(modo ==   CALIBRATOR_COR_ETIQUETADA_SOFT)
	    cor_pixel = calibracaoParam.getSoftColor(ImBruta[offset_v + i][offset_u + j]);
	else
	    cor_pixel = calibracaoParam.getHardColor(ImBruta[offset_v + i][offset_u + j]);

	if(cor_pixel >= 0) count++;
	
	if(count == 0){
	  ImProcessada[i][j] = PxCinza;
	}else if(count == 1){
	  if(modo == CALIBRATOR_IMAGEM_ERROS){
	    ImProcessada[i][j] = PxPreto;
	    //		    *ptProcessada = PxPreto;
	  }else{
	    ImProcessada[i][j] = cores[cor_pixel];
	  }
	}else{
	  ImProcessada[i][j] = PxVermelho;
	}
      }
    }
    break;
  case CALIBRATOR_COR_ATUAL:
    for(i = 0; i < ImProcessada.nlin(); i++){
      for(j = 0; j < ImProcessada.ncol(); j++) {
	cor_pixel = calibracaoParam.getHardColor(ImBruta[offset_v + i][offset_u + j]);
	if(cor_pixel == corAtual){
	  ImProcessada[i][j] = ImBruta[offset_v + i][offset_u + j];
	}else{
	  ImProcessada[i][j] = coresInversas[corAtual];
	}
      }
    }
    break;
  default:
    cerr<<"Qual modo vc espera que eu processe?\n";
    exit(1);
    break;
  }
  return false;
}

void CalibratorProcessor::getPxValor(int x, int y, 
				     int &R, int &G1, int &B, 
				     int &H, int &P, int &G2){
  /*O PROBLEMA ESTA AQUI! 
    
    Por algum motivo ao acessar a ImProcessada para ler os valores e
    setar na interface, é gerado um segmentation fault. Provavelmente
    devido ao acesso a mesma variavel por threads diferentes.
   */
  float myH,myP,myG;
  R = (int)round((ImProcessada[y][x].r/255.0)*100.0);
  G1 = (int)round((ImProcessada[y][x].g/255.0)*100.0);
  B = (int)round((ImProcessada[y][x].b/255.0)*100.0);
  ImProcessada[y][x].getHPG(myH,myP,myG);
  H = (int)round((myH/M_PI)*180);
  P = (int)round(myP*100);
  G2 = (int)round(myG*100);
}

/*
  Fincao para descobrir se um ponto foi selecionado.
  Retorna o indice do ponto selecionado caso exista, e -1 caso nao exista.
*/
int CalibratorProcessor::pontoSelecionado(int u,int v){
  if( !posicaoValida((unsigned int) u, (unsigned int)v) )
    return -1;
    
  int selec = -1;
  unsigned i;
  double dist, menor_dist = hypot((double)LarguraCaptura,(double)AlturaCaptura);	
    
  for(i=0;i<calibracaoParam.nPontosNotaveis;i++){
    dist = hypot(calibracaoParam.pontosImagem[i].u() - (double)(u+offset_u),
		 calibracaoParam.pontosImagem[i].v() - (double)(v+offset_v));
    if(dist <= RAIO_SELECIONADO &&
       dist < menor_dist){
      selec = i;
      menor_dist = dist;
    }
  }
  return selec;
}

/*
  Move o ponto indicado para a posicao (x,y)
*/
void CalibratorProcessor::moverPonto(int ponto,int u,int v){
  if( !posicaoValida((unsigned int) u, (unsigned int)v) )
    return;
  calibracaoParam.pontosImagem[ponto].u() = (double)(u+offset_u);
  calibracaoParam.pontosImagem[ponto].v() = (double)(v+offset_v);
}

/*
  void calibrador::desenharCampoPequeno()
  {
  int i, j, k, count;
  QRgb ponto;
  int cor_pixel = 0;
  bool H_OK = false;
  switch(comboExibicao1->currentItem() ){
  case 0: //Real Image
  imagemGrande_processada = imagem.copy();
  pixmapGrande2.convertFromImage(imagemGrande_processada);
  imagemGrande2Suja = false;
  break;
  case 1: //Current Color Image
  for( i = 0; i < imagem.width(); i++){
  for( j = 0; j < imagem.height(); j++){
  ponto = imagem.pixel(i,j);
  pixel.r = qRed(ponto);
  pixel.g = qGreen(ponto);
  pixel.b = qBlue(ponto);
  pixel.getHPG(H, P_, G_);
  H_OK = false;
  if( limitesHPG[comboCores->currentItem()][0] <= 
  limitesHPG[comboCores->currentItem()][1] ){
  if((int)round(H*100.0) >= limitesHPG[comboCores->currentItem()][0] &&
  (int)round(H*100.0) <= limitesHPG[comboCores->currentItem()][1]){
  H_OK = true;
  }
  }else{
  if((int)round(H*100.0) >= limitesHPG[comboCores->currentItem()][0] ||
  (int)round(H*100.0) <= limitesHPG[comboCores->currentItem()][1]){
  H_OK = true;
  }   
  }
		
  if(H_OK &&
  (int)round(P_*100.0) >= limitesHPG[comboCores->currentItem()][2] &&
  (int)round(P_*100.0) <= limitesHPG[comboCores->currentItem()][3] &&
  (int)round(G_*100.0) >= limitesHPG[comboCores->currentItem()][4] &&
  (int)round(G_*100.0) <= limitesHPG[comboCores->currentItem()][5]){
  imagemGrande_processada.setPixel(i,j,ponto);
  }else{
  if( comboCores->currentItem() != 0 ){
  imagemGrande_processada.setPixel(i,j,qRgb(0,0,0));
  }else{
  imagemGrande_processada.setPixel(i,j,qRgb(255,255,255));
  }
  }
  }
  }
  pixmapGrande2.convertFromImage(imagemGrande_processada);
  break;
  case 2: //Tagged Image
  case 3: //Error Image
  for( i = 0; i < imagem.width(); i++){
  for( j = 0; j < imagem.height(); j++){
  ponto = imagem.pixel(i,j);
  pixel.r = qRed(ponto);
  pixel.g = qGreen(ponto);
  pixel.b = qBlue(ponto);
  pixel.getHPG(H, P_, G_);
  count = 0;
		
  for( k = 0; k < NUM_CORES-NUM_CORES_ADV; k++){
  H_OK = false;
  if( limitesHPG[k][0] <= 
  limitesHPG[k][1] ){
  if((int)round(H*100.0) >= limitesHPG[k][0] &&
  (int)round(H*100.0) <= limitesHPG[k][1]){
  H_OK = true;
  }
  }else{
  if((int)round(H*100.0) >= limitesHPG[k][0] ||
  (int)round(H*100.0) <= limitesHPG[k][1]){
  H_OK = true;
  }   
  }
		    
  if(H_OK &&
  (int)round(P_*100.0) >= limitesHPG[k][2] &&
  (int)round(P_*100.0) <= limitesHPG[k][3] &&
  (int)round(G_*100.0) >= limitesHPG[k][4] &&
  (int)round(G_*100.0) <= limitesHPG[k][5]){
  count++;
  cor_pixel = k;
  }
  }
  if(count == 0){
  imagemGrande_processada.setPixel(i,j,qRgb(127,127,127)); //cinza
  }else if(count == 1){
  if(comboExibicao1->currentItem() == 2){
  switch(cor_pixel){
  case 0:
  imagemGrande_processada.setPixel(i,j,qRgb(0,0,0));
  break;
  case 1:
  imagemGrande_processada.setPixel(i,j,qRgb(255,255,255));
  break;
  case 2:
  imagemGrande_processada.setPixel(i,j,qRgb(0,0,255));
  break;
  case 3:
  imagemGrande_processada.setPixel(i,j,qRgb(255,255,0));	    
  break;
  case 4:
  imagemGrande_processada.setPixel(i,j,qRgb(255,128,0));
  break;
  case 5:
  imagemGrande_processada.setPixel(i,j,qRgb(0,255,255));
  break;
  case 6:
  imagemGrande_processada.setPixel(i,j,qRgb(255,0,255));
  break;
  case 7:
  imagemGrande_processada.setPixel(i,j,qRgb(0,255,0));
  break;
  case 8:
  case 9:
  case 10:
  imagemGrande_processada.setPixel(i,j,qRgb(128,64,0));	        
  break;			    
  }
  }else{
  imagemGrande_processada.setPixel(i,j,qRgb(0,0,0));
  }
  }else{
  imagemGrande_processada.setPixel(i,j,qRgb(255,0,0));
  }
  }
  }
  pixmapGrande2.convertFromImage(imagemGrande_processada);
  break;
  }
    
  //pixmapPequeno1.convertFromImage(imagemPequena);
  pixmapLabelGrande2->setPixmap(pixmapGrande2);	
    
  }
*/
