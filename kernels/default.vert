// default.vert

/*
 This simple GLSL vertex shader does exactly what
 OpenGL would do -- It transforms the vertex positions
 using the default OpenGL transformation. It also passes
 through the texture coordinate, normal coordinate, and some
 other good stuff so we can use them in the fragment shader.
 */

uniform float fullScreenQuad;


// This 'varying' vertex output can be read as an input
// by a fragment shader that makes the same declaration.
varying vec3 modelPos;
//varying vec3 lightSourcePos;
varying vec3 normal;
varying vec2 texPos;

void main()
{


    // Render the shape using standard OpenGL position transforms.
    gl_Position = ftransform();

    // Copy the standard OpenGL texture coordinate to the output.
    texPos = gl_MultiTexCoord0.xy;

    normal = gl_Normal.xyz;
    modelPos = gl_Vertex.xyz;

    if (fullScreenQuad < 0.0){

        // Render the shape using modified position.
        gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix *  vec4(modelPos,1);

        // we may need this in the fragment shader...
        modelPos = (gl_ModelViewMatrix * vec4(modelPos,1)).xyz;

        // send the normal to the fragment shader
        normal = normalize((gl_NormalMatrix * normal).xyz);
    } else {
        // I can't believe this works
        gl_Position = vec4(texPos.x - .5, texPos.y -.5 , 0, .5);
    }

    // pass the light source position to the fragment shader
    //    lightSourcePos = gl_LightSource[0].position.xyz;
}