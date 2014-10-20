// phong.frag

/*
 This fragment implements the Phong Reflection model.
 */

// The input image we will be filtering in this kernel.
uniform sampler2D colorTex;

uniform float colorMapping;
uniform float normalFlipping;
uniform float glowMapping;
uniform float fullScreenQuad;
uniform float blur;
uniform float blurV;
uniform float blend;

const float blurSize = 1.0/512.0;

varying vec3 modelPos;    // fragment position in model space
varying vec2 texPos;      // fragment position in texture space
                          //varying vec3 lightSourcePos; // light source position in model space
varying vec3 normal;	  // fragment normal in model space

void main()
{
    // Sample from the normal map, if we're not doing displacement mapping
    vec3 N, C;
    float att, dist;
    vec3 materialAmbient;
    vec3 materialDiffuse;
    vec3 materialSpecular;
    vec3 lightAmbient, lightDiffuse, lightSpecular;
    vec3 Ld, Lm, Rm, V, colorAmbient, colorDiffuse, colorSpecular;
    vec3 color;

    if (fullScreenQuad > 0.0){
        if (blur > 0.0){
            vec4 sum = vec4(0.0);
            if (blurV > 0.0){
                // blur in y (vertical)
                // take nine samples, with the distance blurSize between them
                sum += texture2D(colorTex, vec2(texPos.x - 4.0*blurSize, texPos.y)) * 0.05;
                sum += texture2D(colorTex, vec2(texPos.x - 3.0*blurSize, texPos.y)) * 0.09;
                sum += texture2D(colorTex, vec2(texPos.x - 2.0*blurSize, texPos.y)) * 0.12;
                sum += texture2D(colorTex, vec2(texPos.x - blurSize, texPos.y)) * 0.15;
                sum += texture2D(colorTex, vec2(texPos.x, texPos.y)) * 0.16;
                sum += texture2D(colorTex, vec2(texPos.x + blurSize, texPos.y)) * 0.15;
                sum += texture2D(colorTex, vec2(texPos.x + 2.0*blurSize, texPos.y)) * 0.12;
                sum += texture2D(colorTex, vec2(texPos.x + 3.0*blurSize, texPos.y)) * 0.09;
                sum += texture2D(colorTex, vec2(texPos.x + 4.0*blurSize, texPos.y)) * 0.05;
            } else {
                sum += texture2D(colorTex, vec2(texPos.x, texPos.y - 4.0*blurSize)) * 0.05;
                sum += texture2D(colorTex, vec2(texPos.x, texPos.y - 3.0*blurSize)) * 0.09;
                sum += texture2D(colorTex, vec2(texPos.x, texPos.y - 2.0*blurSize)) * 0.12;
                sum += texture2D(colorTex, vec2(texPos.x, texPos.y - blurSize)) * 0.15;
                sum += texture2D(colorTex, vec2(texPos.x, texPos.y)) * 0.16;
                sum += texture2D(colorTex, vec2(texPos.x, texPos.y + blurSize)) * 0.15;
                sum += texture2D(colorTex, vec2(texPos.x, texPos.y + 2.0*blurSize)) * 0.12;
                sum += texture2D(colorTex, vec2(texPos.x, texPos.y + 3.0*blurSize)) * 0.09;
                sum += texture2D(colorTex, vec2(texPos.x, texPos.y + 4.0*blurSize)) * 0.05;
            }
            //            return;
            //            gl_FragColor = clamp(sum, 0.0, 1.0);
            gl_FragColor = sum;
            return;
        } else {
            color = gl_FrontMaterial.ambient.xyz*texture2D(colorTex, texPos).xyz;
            color = clamp(color, 0.0, 1.0);
            if (blend > 0.0){
                //                gl_FragColor = vec4(1.0, 0, 0, 1.0);
                gl_FragColor = vec4(color, .01);
            } else {
                gl_FragColor = vec4(color, 1.0);
            }
            return;
        }
    }


    N = normal;
    N=normalize(N);

    if (normalFlipping > 0.0){
        N.x *= -1.0;
        N.y *= -1.0;
        N.z *= -1.0;
    }

    C = vec3(0.0, 0.0, 0.0); // camera position

    if(colorMapping < 0.0){
        materialAmbient = gl_FrontMaterial.ambient.xyz;
        materialDiffuse = gl_FrontMaterial.diffuse.xyz;
        materialSpecular  = gl_FrontMaterial.specular.xyz;
    } else{
        materialAmbient = gl_FrontMaterial.ambient.xyz*texture2D(colorTex, texPos).xyz;
        //		materialDiffuse = gl_FrontMaterial.diffuse.xyz*texture2D(colorTex, texPos).xyz;
        materialDiffuse = gl_FrontMaterial.diffuse.xyz*texture2D(colorTex, texPos).xyz;
        materialSpecular  = gl_FrontMaterial.specular.xyz*texture2D(colorTex, texPos).xyz;
    }
    float shininess    = gl_FrontMaterial.shininess;

    color = vec3(0.0,0.0,0.0);
    for (int i = 0; i < 8; i++) {
        lightAmbient  = gl_LightSource[i].ambient.xyz;

        if (glowMapping > 0.0){
            lightAmbient  = vec3(1.0,1.0,1.0);
        }

        lightDiffuse  = gl_LightSource[i].diffuse.xyz;
        lightSpecular = gl_LightSource[i].specular.xyz;

        //        vec3 Lm = normalize(lightSourcePos-modelPos);
        Ld = gl_LightSource[i].position.xyz-modelPos;
        Lm = normalize(Ld);

        dist = length(Ld);// / 30.0;

        //        float dist = 1.0;

        Rm = normalize(reflect(-Lm,N));
        V = normalize(-modelPos);

        colorAmbient = materialAmbient*lightAmbient;
        colorDiffuse = clamp(max(dot(Lm,N),0.0)*materialDiffuse*lightDiffuse,0.0,1.0);
        colorSpecular = clamp(pow(max(dot(Rm,V),0.0),shininess)*materialSpecular*lightSpecular,0.0,1.0);
        if (glowMapping < 0.0){
            att = 1.0 / (gl_LightSource[i].constantAttenuation + gl_LightSource[i].linearAttenuation * dist + gl_LightSource[i].quadraticAttenuation * dist * dist);
            color += (colorAmbient) + att* colorDiffuse + att * colorSpecular;

        } else {
            // I need to filter out the darker colors...
//            color += colorAmbient + colorDiffuse;
            color += colorAmbient + colorDiffuse + colorSpecular;

        }
    }

    if (glowMapping < 0.0){
        color = clamp(color, 0.0, 1.0);
        gl_FragColor = vec4(color, 1.0);
    } else {
        // IF glowmapping
        vec3 dark = vec3(.4,.4,.4);
        vec3 white = vec3(.99, .99, .99);
        color = clamp(color, 0.0, 1.0);
        if (length(color) < length(dark)){
            // alpha out all the things we don't want
            color = vec3(0,0,0);
            gl_FragColor = vec4(0,0,0,0);
        } else {
            color = clamp(color, 1.0, 1.0);
            gl_FragColor = vec4(color, 1.0);
        }
    }


    //        color += (colorAmbient) + (colorDiffuse / dist) + (colorSpecular/dist);
    //                color += (colorAmbient * 1.0/d) + (colorDiffuse * 1.0/d) + (colorSpecular * 1.0/d);
    //                color += colorAmbient + colorDiffuse + colorSpecular;
    
    
    
    
}
