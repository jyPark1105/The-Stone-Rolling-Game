#version 430 core
in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos; // ī�޶� ��ġ

out vec4 FragColor;

void main()
{
    vec3 viewDir = normalize(viewPos - FragPos);
    float edgeFactor = dot(normalize(Normal), viewDir);

    // ������ ����� ��츸 �׸���
    if (abs(edgeFactor) < 0.15)
        FragColor = vec4(0.0, 0.6, 1.0, 1.0); // �Ķ� �ܰ���
    else
        discard; // �� ���̰� ����
}
