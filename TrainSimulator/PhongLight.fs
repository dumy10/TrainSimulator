//----------------------------------------------------------------------------------------------
// PhongLight.fs
  #version 330 core
  out vec4 FragColor;

  in vec3 Normal;  
  in vec3 FragPos;  

  uniform vec3 lightPos; 
  uniform vec3 viewPos; 
  uniform vec3 lightColor;
  uniform vec3 objectColor;

  uniform float Ka; // ambient coefficient
  uniform float Kd; // diffuse coefficient
  uniform float Ks; // specular coefficient
  uniform float shininess;

  void main()
  {
      vec3 ambient = Ka * lightColor;
      vec3 norm = normalize(Normal);

      vec3 lightDir = normalize(lightPos - FragPos);
      float diff = max(dot(norm, lightDir), 0.0);

      vec3 diffuse = Kd * diff * lightColor;
      vec3 viewDir = normalize(viewPos - FragPos);

      vec3 reflectDir = reflect(-lightDir, norm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
      vec3 specular = Ks * spec * lightColor;

      vec3 result = (ambient + diffuse + specular) * objectColor;

      FragColor = vec4(result, 1.0);
  }