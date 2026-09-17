= Trình dò tia trên CPU
== Mục tiêu
- Xây dựng được một trình dò tia đơn giản dựa vào các định luật quang học cơ bản
- Kết xuất được các vật thể là hình cầu, khối và các vật thể phức tạp tạo từ nhiều hình tam giác khác nhau
- Kết xuất được hình phản chiếu của các vật thể trên bề mặt gương
- Cho thấy rõ hiệu ứng ánh sáng toàn cục
- Có trình chỉnh sửa đơn giản:
  - Tùy chỉnh màu sắc, chiết xuất, cường độ sáng, ... của vật thể
  - Thay đổi vị trí, góc quay của vật thể
  - Thay đổi điểm nhìn, góc nhìn
- Cho phép xuất ảnh, xuất cảnh dựng
== Tổng quan các lớp
- RNG
  - float get_uniform(float from=0, float to=1)
  - float get_normal(float std=0, float dev=1)
  - Vector3 get()

- Vector
  - const size
  - float data[]
- Vector3: Vector
- Vector2: Vector

- Color: Vector3
  - std::uint32_t get_abgr()

- AABB
  - Vector3 min
  - Vector3 max

- Camera
  - Vector3 position
  - float pan
  - float tilt

- Texture
  - virtual Color get(HitInfo hp)

- ImageTexture: Texture
  - std::uint8_t\* image

- ColorTexture: Texture
  - Color color

- Material
  - float roughness
  - float ior
  - float emission_strength
  - Texture\* texture

- Ray
  - Vector3 origin
  - Vector3 direction
  - Color color

- HitInfo
  - Vector3 hit_point
  - Vector3 normal
  - Vector2 uv
  - bool front_face
  - Hittable\* object

- Hittable:
  - Vector3 position
  - Vector3 rotation
  - Material\* material
  - virtual void translate(Vector3 new_position)
  - virtual void rotate(Vector3 new_rotation)
  - virtual HitPoint intersect(Ray ray)
  - virtual AABB get_aabb()

- Sphere: Hittable
  - float radius

- Triangle: Hittable
  - Vector3 points[3]

- BVHNode
  - AABB aabb
  - BVH\* left
  - BVH\* right
  - std::vector\<Hittable&\> object
  - bool is_leaf()

- BVH
  - BVHNode\* root
  - BVH\* build(const std::vector\<Hittable\>& scene)

- Scene
  - std::vector\<Hittable\> objects
  - int add_object(Hittable obj)
  - int remove_object(int id)
  - void save_scene(std::string path)

- RayTracer
  - Camera camera
  - Scene scene
  - BVH bhv
  - std::uint32_t\* data
  - void render()

- App
  - RayTracer ray_tracer
  - void ui_draw()
  - void save_image(std::string path)

== Cấu trúc dữ liệu
=== Cây tìm kiếm BVH (Bounding Volume Hierachy)
Thay vì duyệt tuyến tính trên danh sách các vật thể để xem tia hiện tại cắt vật nào gần nhất thì xây dựng cây nhị phân tìm kiếm để duyệt nhanh hơn.

Cách xây:
+ Chia các vật thể thành 2 nhóm có thể tích xấp xỉ nhau, 2 nhóm này sẽ tạo ra 2 nút con trên nút gốc ban đầu.
+ Tếp tục chia như vậy cho đến khi số vật thể trong nhóm không lớn hơn 1 giới hạn nhất định. Khi này nút cuối cùng của 1 nhánh sẽ chứa nhóm vật thể này và là nút lá.

Khi tiến hành dò tia, chỉ cần kiểm tra xem tia có đi qua AABB của 1 nhóm hay không. Nếu có thì tiếp tục xét 2 nút con của nút này. Cứ như vậy cho đến khi duyệt đến nút lá rồi tiến hành duyệt tuần tự các vật thể trong nhóm.

Lý do nút lá phải chứa ít nhất 1 số vật thể: việc kiểm tra xem 1 tia có đi qua 1 AABB không cũng tốn kém. Việc có 1 số tối đa các vật thể trong cùng 1 nút lá giúp cây được xây không quá sâu (giảm số lần kiểm tra cắt tia), đảm bảo rằng chi phí duyệt cây không lớn hơn chi phí duyệt tuyến tính.

Nếu chi phí kiểm tra xem tia có cắt 1 vật thể không là $t$, chi phí để kiểm tra xem tia có cắt 1 AABB không là $w$, số vật thể tối đa trong 1 nút là $k$, số vật thể là $n$
thì:
- chi phí duyệt tuyến tính thuần: $t n$
- chi phí duyệt cây: $2 w log_2(n / k) + t k$
Dễ thấy rằng với giá trị k tối ưu thì chi phí duyệt giảm đi rất nhiều lần và k ít nhất phải lớn hơn 1.
