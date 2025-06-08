#version 430 core
in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos; // 카메라 위치

out vec4 FragColor;

void main()
{
    vec3 viewDir = normalize(viewPos - FragPos);
    float edgeFactor = dot(normalize(Normal), viewDir);

    // 수직에 가까운 경우만 그리기
    if (abs(edgeFactor) < 0.15)
        FragColor = vec4(0.0, 0.6, 1.0, 1.0); // 파란 외곽선
    else
        discard; // 안 보이게 버림
}
