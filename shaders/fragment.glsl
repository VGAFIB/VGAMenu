uniform float time;

varying vec3 vColor;
varying vec2 pos;

const float Pi = 3.14159;
const int zoom = 40;
const float speed = 0.01;
float fScale = 1.25;

void main(void)
{
    float speed2 = 30.0;

    vec2 uv = pos;
    vec2 p=pos*2-1;

    float ct = time * speed;

    for(int i=1;i<zoom;i++) {
        vec2 newp=p;
        newp.x+=0.25/float(i)*cos(float(i)*p.y+time*speed2*cos(ct)*0.3/40.0+0.03*float(i))*fScale+10.0;
        newp.y+=0.5/float(i)*cos(float(i)*p.x+time*speed2*ct*0.3/50.0+0.03*float(i+10))*fScale+15.0;
        p=newp;
    }

    vec3 col=vec3(0.5*sin(3.0*p.x)+0.5,0.5*sin(3.0*p.y)+0.5,sin(p.x+p.y));
    gl_FragColor=vec4(col*vColor*0.6+vColor*0.4, 1.0);
}


/*uniform float time;

varying vec3 vColor;
varying vec2 pos;

void main(void)
{
    vec3 color = vColor;
    color += sin(time)*0.1+vec3(pos, 0.0);
    gl_FragColor = vec4(color, 1.0)*0.7+0.3*vec4(sin(pos*30+sin(vec2(time*sin(time), pos.x+time))*10+time+sin(pos.x*100+time*30))*0.5+0.5, 0.0, 1.0);
}
*/
