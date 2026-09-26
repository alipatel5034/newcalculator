#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159265358979323846

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define API EMSCRIPTEN_KEEPALIVE
#else
#define API
#endif

static char out[8192];

static void setout(const char *s){ snprintf(out,sizeof(out),"%s",s); }
static const char *fmt1(double x){ snprintf(out,sizeof(out),"%.10g",x); return out; }

API const char* web_last_output(void){ return out; }

API const char* web_basic(double a, int op, double b){
    double r=0;
    if(op==1) r=a+b;
    else if(op==2) r=a-b;
    else if(op==3) r=a*b;
    else if(op==4){ if(b==0){setout("Error: Division by zero.");return out;} r=a/b; }
    else if(op==5) r=pow(a,b);
    else {setout("Invalid operation.");return out;}
    return fmt1(r);
}

API const char* web_unary(double x,int op){
    double r=0;
    switch(op){
        case 1:r=sin(x*PI/180.0);break;
        case 2:r=cos(x*PI/180.0);break;
        case 3:
            if(fabs(cos(x*PI/180.0))<1e-12){setout("tan is undefined at this angle.");return out;}
            r=tan(x*PI/180.0);break;
        case 4: if(x<-1||x>1){setout("asin domain error: x must be between -1 and 1.");return out;} r=asin(x)*180.0/PI;break;
        case 5: if(x<-1||x>1){setout("acos domain error: x must be between -1 and 1.");return out;} r=acos(x)*180.0/PI;break;
        case 6:r=atan(x)*180.0/PI;break;
        case 7:if(x<0){setout("sqrt domain error.");return out;}r=sqrt(x);break;
        case 8:if(x<=0){setout("log domain error.");return out;}r=log10(x);break;
        case 9:if(x<=0){setout("ln domain error.");return out;}r=log(x);break;
        case 10:if(x==0){setout("Cannot calculate 1/0.");return out;}r=1/x;break;
        case 11:r=x*x;break;
        default:setout("Invalid operation.");return out;
    }
    return fmt1(r);
}

API const char* web_complex(double ar,double ai,double br,double bi,int op){
    double rr=0,ii=0;
    if(op==1){rr=ar+br;ii=ai+bi;}
    else if(op==2){rr=ar-br;ii=ai-bi;}
    else if(op==3){rr=ar*br-ai*bi;ii=ar*bi+ai*br;}
    else if(op==4){
        double d=br*br+bi*bi;
        if(d==0){setout("Error: complex division by zero.");return out;}
        rr=(ar*br+ai*bi)/d;ii=(ai*br-ar*bi)/d;
    }else if(op==5){
        double mag=sqrt(ar*ar+ai*ai),arg=atan2(ai,ar)*180/PI;
        snprintf(out,sizeof(out),"Magnitude = %.10g\nArgument = %.10g degrees",mag,arg);
        return out;
    }else{setout("Invalid operation.");return out;}
    snprintf(out,sizeof(out),"%.10g %c %.10gi",rr,(ii>=0?'+':'-'),fabs(ii));
    return out;
}

API const char* web_linear2(double a1,double b1,double c1,double a2,double b2,double c2){
    double d=a1*b2-a2*b1;
    if(fabs(d)<1e-12){setout("No unique solution.");return out;}
    double x=(c1*b2-c2*b1)/d;
    double y=(a1*c2-a2*c1)/d;
    snprintf(out,sizeof(out),"X = %.10g\nY = %.10g",x,y);return out;
}

API const char* web_linear3(
    double a1,double b1,double c1,double d1,
    double a2,double b2,double c2,double d2,
    double a3,double b3,double c3,double d3){
    double D=a1*(b2*c3-b3*c2)-b1*(a2*c3-a3*c2)+c1*(a2*b3-a3*b2);
    double Dx=d1*(b2*c3-b3*c2)-b1*(d2*c3-d3*c2)+c1*(d2*b3-d3*b2);
    double Dy=a1*(d2*c3-d3*c2)-d1*(a2*c3-a3*c2)+c1*(a2*d3-d3*a2);
    double Dz=a1*(b2*d3-b3*d2)-b1*(a2*d3-a3*d2)+d1*(a2*b3-a3*b2);
    if(fabs(D)<1e-12){setout("No unique solution.");return out;}
    snprintf(out,sizeof(out),"X = %.10g\nY = %.10g\nZ = %.10g",Dx/D,Dy/D,Dz/D);return out;
}

API const char* web_quadratic(double a,double b,double c){
    if(fabs(a)<1e-12){setout("a cannot be zero.");return out;}
    double D=b*b-4*a*c;
    if(D>1e-12){
        double r1=(-b+sqrt(D))/(2*a),r2=(-b-sqrt(D))/(2*a);
        snprintf(out,sizeof(out),"X1 = %.10g\nX2 = %.10g",r1,r2);
    }else if(fabs(D)<=1e-12){
        snprintf(out,sizeof(out),"X = %.10g",-b/(2*a));
    }else{
        double re=-b/(2*a),im=sqrt(-D)/(2*fabs(a));
        snprintf(out,sizeof(out),"X1 = %.10g + %.10gi\nX2 = %.10g - %.10gi",re,im,re,im);
    }
    return out;
}

API const char* web_cubic(double a,double b,double c,double d){
    if(fabs(a)<1e-12){setout("Not a cubic equation.");return out;}
    /* Cardano / trigonometric real-root handling for common cases */
    double A=b/a,B=c/a,C=d/a;
    double p=B-A*A/3.0;
    double q=2*A*A*A/27.0-A*B/3.0+C;
    double disc=q*q/4.0+p*p*p/27.0;
    if(disc>1e-12){
        double u=cbrt(-q/2+sqrt(disc)),v=cbrt(-q/2-sqrt(disc));
        double x=u+v-A/3;
        snprintf(out,sizeof(out),"One real root:\nX1 = %.10g",x);
    }else if(fabs(disc)<=1e-12){
        double u=cbrt(-q/2);
        double x1=2*u-A/3,x2=-u-A/3;
        snprintf(out,sizeof(out),"Real roots:\nX1 = %.10g\nX2 = %.10g",x1,x2);
    }else{
        double r=2*sqrt(-p/3);
        double theta=acos((3*q/(2*p))*sqrt(-3/p));
        double x1=r*cos(theta/3)-A/3;
        double x2=r*cos((theta+2*PI)/3)-A/3;
        double x3=r*cos((theta+4*PI)/3)-A/3;
        snprintf(out,sizeof(out),"Three real roots:\nX1 = %.10g\nX2 = %.10g\nX3 = %.10g",x1,x2,x3);
    }
    return out;
}

API const char* web_base(long long n,int base){
    if(base==2) snprintf(out,sizeof(out),"%lld",n<0?0:n);
    if(base==8) snprintf(out,sizeof(out),"%llo",(unsigned long long)n);
    if(base==16) snprintf(out,sizeof(out),"%llX",(unsigned long long)n);
    if(base==2){
        if(n==0){setout("0");return out;}
        unsigned long long u=(unsigned long long)n; char t[128];int i=0;
        while(u){t[i++]=(u&1)+'0';u>>=1;}
        int j=0;while(i--)out[j++]=t[i];out[j]=0;
    }
    return out;
}

static double det2(double*a){return a[0]*a[3]-a[1]*a[2];}
static double det3(double*a){
    return a[0]*(a[4]*a[8]-a[5]*a[7])-a[1]*(a[3]*a[8]-a[5]*a[6])+a[2]*(a[3]*a[7]-a[4]*a[6]);
}

API const char* web_matrix(int n,int op,const double*A,const double*B){
    double R[9]={0},d;
    if(n==2){
        d=det2((double*)A);
        if(op==1) snprintf(out,sizeof(out),"Determinant = %.10g",d);
        else if(op==2) snprintf(out,sizeof(out),"[%g %g]\n[%g %g]",A[0],A[2],A[1],A[3]);
        else if(op==3){
            if(fabs(d)<1e-12){setout("Inverse does not exist (determinant = 0).");return out;}
            R[0]=A[3]/d;R[1]=-A[1]/d;R[2]=-A[2]/d;R[3]=A[0]/d;
            snprintf(out,sizeof(out),"[%g %g]\n[%g %g]",R[0],R[1],R[2],R[3]);
        } else {
            for(int i=0;i<4;i++) R[i]=(op==4?A[i]+B[i]:op==5?A[i]-B[i]:0);
            if(op==6){R[0]=A[0]*B[0]+A[1]*B[2];R[1]=A[0]*B[1]+A[1]*B[3];R[2]=A[2]*B[0]+A[3]*B[2];R[3]=A[2]*B[1]+A[3]*B[3];}
            snprintf(out,sizeof(out),"[%g %g]\n[%g %g]",R[0],R[1],R[2],R[3]);
        }
    }else{
        d=det3((double*)A);
        if(op==1) snprintf(out,sizeof(out),"Determinant = %.10g",d);
        else if(op==2) snprintf(out,sizeof(out),"[%g %g %g]\n[%g %g %g]\n[%g %g %g]",A[0],A[3],A[6],A[1],A[4],A[7],A[2],A[5],A[8]);
        else if(op==3){
            if(fabs(d)<1e-12){setout("Inverse does not exist (determinant = 0).");return out;}
            double cof[9];
            cof[0]=A[4]*A[8]-A[5]*A[7]; cof[1]=-(A[3]*A[8]-A[5]*A[6]); cof[2]=A[3]*A[7]-A[4]*A[6];
            cof[3]=-(A[1]*A[8]-A[2]*A[7]); cof[4]=A[0]*A[8]-A[2]*A[6]; cof[5]=-(A[0]*A[7]-A[1]*A[6]);
            cof[6]=A[1]*A[5]-A[2]*A[4]; cof[7]=-(A[0]*A[5]-A[2]*A[3]); cof[8]=A[0]*A[4]-A[1]*A[3];
            for(int i=0;i<3;i++)for(int j=0;j<3;j++)R[i*3+j]=cof[j*3+i]/d;
            snprintf(out,sizeof(out),"[%g %g %g]\n[%g %g %g]\n[%g %g %g]",R[0],R[1],R[2],R[3],R[4],R[5],R[6],R[7],R[8]);
        }else{
            if(op==4||op==5)for(int i=0;i<9;i++)R[i]=(op==4?A[i]+B[i]:A[i]-B[i]);
            if(op==6)for(int i=0;i<3;i++)for(int j=0;j<3;j++)for(int k=0;k<3;k++)R[i*3+j]+=A[i*3+k]*B[k*3+j];
            snprintf(out,sizeof(out),"[%g %g %g]\n[%g %g %g]\n[%g %g %g]",R[0],R[1],R[2],R[3],R[4],R[5],R[6],R[7],R[8]);
        }
    }
    return out;
}

API const char* web_vector(int n,int op,double ax,double ay,double az,double bx,double by,double bz){
    if(op==1) snprintf(out,sizeof(out),"(%g, %g%s)",ax+bx,ay+by,n==3?", z":"");
    else if(op==2) snprintf(out,sizeof(out),"(%g, %g%s)",ax-bx,ay-by,n==3?", z":"");
    else if(op==3){double d=ax*bx+ay*by+(n==3?az*bz:0);snprintf(out,sizeof(out),"Dot Product = %.10g",d);}
    else if(op==4 && n==2) snprintf(out,sizeof(out),"Cross Product (scalar) = %.10g",ax*by-ay*bx);
    else if(op==4 && n==3) snprintf(out,sizeof(out),"(%g, %g, %g)",ay*bz-az*by,az*bx-ax*bz,ax*by-ay*bx);
    else setout("Invalid vector operation.");
    return out;
}
