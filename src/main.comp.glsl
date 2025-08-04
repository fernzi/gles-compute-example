#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0) buffer SSBO
{
  uint values[];
};

layout(location = 0) uniform uint elements;

uint fibonacci(uint n)
{
  if (n <= 1u) {
    return n;
  }
  uint curr = 1u;
  uint prev = 1u;
  for (uint i = 2u; i < n; ++i) {
    uint temp = curr;
    curr += prev;
    prev = temp;
  }
  return curr;
}

void main()
{
  uint index = gl_GlobalInvocationID.x;
  if (index >= elements) {
    return;
  }
  values[index] = fibonacci(values[index]);
}
