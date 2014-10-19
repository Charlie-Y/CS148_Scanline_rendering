// phong.frag

/*
  This fragment implements the Phong Reflection model.
*/

// The input image we will be filtering in this kernel.
uniform sampler2D normalTex;
uniform sampler2D colorTex;

uniform float normalMapping;
uniform float colorMapping;
uniform float normalFlipping;

varying vec3 modelPos;    // fragment position in model space
varying vec2 texPos;      // fragment position in texture space
//varying vec3 lightSourcePos; // light source position in model space
varying vec3 normal;	  // fragment normal in model space

void main()
{
    // Sample from the normal map, if we're not doing displacement mapping
    vec3 N;
    float att, dist;

    N = normal;
    N=normalize(N);

    if (normalFlipping > 0.0){
        N.x *= -1.0;
        N.y *= -1.0;
        N.z *= -1.0;
    }
    
    vec3 C = vec3(0.0, 0.0, 0.0); // camera position


	vec3 materialAmbient;
	vec3 materialDiffuse;
	vec3 materialSpecular;
	if(colorMapping < 0.0){
		materialAmbient = gl_FrontMaterial.ambient.xyz;
		materialDiffuse = gl_FrontMaterial.diffuse.xyz;
		materialSpecular  = gl_FrontMaterial.specular.xyz;
	}
	else{
		materialAmbient = gl_FrontMaterial.ambient.xyz*texture2D(colorTex, texPos).xyz;
//		materialDiffuse = gl_FrontMaterial.diffuse.xyz*texture2D(colorTex, texPos).xyz;
        materialDiffuse = gl_FrontMaterial.diffuse.xyz*texture2D(colorTex, texPos).xyz;
		materialSpecular  = gl_FrontMaterial.specular.xyz*texture2D(colorTex, texPos).xyz;
	}
    float shininess    = gl_FrontMaterial.shininess;

    vec3 color = vec3(0.0,0.0,0.0);
    for (int i = 0; i < 8; i++) {
        vec3 lightAmbient  = gl_LightSource[i].ambient.xyz;
        vec3 lightDiffuse  = gl_LightSource[i].diffuse.xyz;
        vec3 lightSpecular = gl_LightSource[i].specular.xyz;

//        vec3 Lm = normalize(lightSourcePos-modelPos);
        vec3 Ld = gl_LightSource[i].position.xyz-modelPos;
        vec3 Lm = normalize(Ld);

        dist = length(Ld);// / 30.0;

//        float dist = 1.0;

        vec3 Rm = normalize(reflect(-Lm,N));
        vec3 V = normalize(-modelPos);

        vec3 colorAmbient = materialAmbient*lightAmbient;
        vec3 colorDiffuse = clamp(max(dot(Lm,N),0.0)*materialDiffuse*lightDiffuse,0.0,1.0);
        vec3 colorSpecular = clamp(pow(max(dot(Rm,V),0.0),shininess)*materialSpecular*lightSpecular,0.0,1.0);

        att = 1.0 / (gl_LightSource[i].constantAttenuation + gl_LightSource[i].linearAttenuation * dist +                     gl_LightSource[i].quadraticAttenuation * dist * dist);

        color += (colorAmbient) + att* colorDiffuse + att * colorSpecular;

//        color += (colorAmbient) + (colorDiffuse / dist) + (colorSpecular/dist);
//                color += (colorAmbient * 1.0/d) + (colorDiffuse * 1.0/d) + (colorSpecular * 1.0/d);
        //                color += colorAmbient + colorDiffuse + colorSpecular;
    }
    
	color = clamp(color, 0.0, 1.0);
    gl_FragColor = vec4(color, 1.0);



}
