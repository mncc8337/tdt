#import "@preview/fletcher:0.5.8" as fletcher: diagram, node, edge

#let class(name, attributes, methods) = block(
  stroke: 0.5pt,
  radius: 2pt,
  inset: 0pt,
  width: auto,
  [
    #set block(spacing: 2pt)
    #pad(5pt)[#align(center)[#text(font: "CaskaydiaCove NF")[*#name*]]]
    #if attributes != none [
      #line(length: 100%, stroke: 0.5pt)
      #pad(5pt)[#align(left)[#text(size: 7pt, font: "CaskaydiaCove NF")[#attributes.join("\n")]]]
    ]
    #if methods != none [
      #line(length: 100%, stroke: 0.5pt)
      #pad(5pt)[#align(left)[#text(size: 7pt, font: "CaskaydiaCove NF")[#methods.join("\n")]]]
    ]
  ]
)
#let edge = edge.with(mark-scale: 200%)
#import fletcher.cetz.draw
#let inheritance = (none, (
  length: 1,
  width: 1,
  draw: draw.line(
    (0, 0),
    (-5, 2.5),
    (-5, -2.5),
    close: true,
    fill: white,
    stroke: 0.5pt
  )
))
#let composition = ((inherit: "diamond", fill: black), none)
#let aggregation = ((inherit: "diamond"), none)

#set page(margin: (
  top: 1cm,
  bottom: 1cm,
  x: 1.5cm,
))

= Trình dò tia trên CPU
== Mục tiêu
- Vận dụng kiến thức Lập trình hướng đối tượng để thiết kế kiến trúc phần mềm linh hoạt, có tính đóng gói cao và dễ dàng mở rộng các loại vật thể, vật liệu mới.
- Ứng dụng Cấu trúc dữ liệu (cụ thể là cây nhị phân tìm kiếm) để giải quyết bài toán tối ưu hóa hiệu năng, giảm thiểu thời gian tính toán giao cắt trong không gian.
- Xây dựng thành công một trình dò tia trên CPU có khả năng mô phỏng các định luật quang học và kết xuất hình ảnh chân thực.

== Nhiệm vụ
=== Về tính toán

- Cài đặt hệ thống hình học toán học cơ bản (Vector, Tia, Hộp bao AABB, Hình cầu, Tam giác).
- Xử lý tính toán giao cắt tia với các hình học cơ bản (mặt cầu) và phức tạp (lưới đa giác tạo từ nhiều tam giác).
- Mô phỏng các tính chất vật liệu quang học: phản xạ gương (kim loại), khúc xạ (thủy tinh/nước) và tán xạ khuếch tán (bề mặt nhám).
- Tính toán ánh sáng toàn cục bằng thuật toán Path Tracing.

=== Về cấu trúc dữ liệu

- Cài đặt cấu trúc dữ liệu cây BVH (Bounding Volume Hierarchy) để tăng tốc độ truy vấn giao điểm giữa tia và hàng ngàn vật thể trong không gian.
- Ứng dụng đa luồng để phân chia công việc tính toán các điểm ảnh lên nhiều lõi CPU.

=== Về giao diện

- Xây dựng không gian làm việc trực quan.
- Xây dựng các chức năng tương tác thời gian thực:
  - Thay đổi thuộc tính vật thể (vị trí, góc quay, màu sắc, chiết suất, độ nhám, cường độ sáng).
  - Điều hướng Camera (vị trí, góc nhìn, tiêu cự).
- Cung cấp tính năng lưu/tải cảnh và xuất kết quả kết xuất ra tệp tin ảnh PNG/JPG.

== Thư viện sử dụng
- Thư viện đồ họa SFML kết hợp với thư viện giao diện người dùng ImGUI
- Thư viện đa luồng `std::thread` có sẵn của C++11
- Thư viện mảng động `std::vector` có sẵn của C++
== Cấu trúc dữ liệu
=== Tổng quan các lớp

// #pagebreak()

#align(center)[
  #diagram(
    node-shape: rect,
    spacing: (0.05cm, 0.05cm),

    node((-1, -1), class(
      "App",
      ("- ray_tracer: RayTracer",),
      (
        "+ ui_draw()",
        "+ save_image(path: string)"
      )
    ), name: <app>),

    node((0, -1), class(
      "RayTracer", 
      (
        "- camera: Camera",
        "- scene: Scene",
        "- data: uint32_t*"
      ), 
      (
        "+ render()",
      )
    ), name: <raytracer>),

    node((1, -1), class(
      "Camera", 
      (
        "- position: Vector3",
        "- pan: float",
        "- tilt: float"
      ),
      (
        "+ get_ray(uv: Vector2): Ray",
      )
    ), name: <camera>),

    node((-1, 0), class(
      "Scene", 
      (
        "- objects: vector<unique_ptr<Hittable>>",
        "- materials: vector<unique_ptr<Material>>",
        "- textures: vector<unique_ptr<Texture>>",
        "- bvh_root: unique_ptr<BVHNode>"
      ), 
      (
        "+ add_object(obj: unique_ptr<Hittable>)",
        "+ remove_object(id: int): int",
        "+ save_scene(path: string)",
        "+ load_scene(path: string): Scene*",
        "+ build_bvh()"
      )
    ), name: <scene>),

    node((-1, 1.5), class(
      "BVHNode", 
      (
        "- aabb: AABB",
        "- left: unique_ptr<BVHNode>",
        "- right: unique_ptr<BVHNode>",
        "- object: vector<Hittable*>"
      ), 
      (
        "+ is_leaf(): bool",
      )
    ), name: <bvhnode>),

    node((0, 3.5), class(
      "Hittable", 
      (
        "- position: Vector3",
        "- rotation: Vector3",
        "- material: Material*"
      ), 
      (
        "+ translate(new_pos: Vector3)",
        "+ rotate(new_rot: Vector3)",
        "+ intersect(ray: Ray&, rec: HitInfo&): bool",
        "+ get_aabb(): AABB"
      )
    ), name: <hittable>),

    node((0, 0.9), class(
      "Material", 
      (
        "- roughness: float",
        "- ior: float",
        "- emission_strength: float",
        "- texture: Texture*"
      ), 
      (
        "+ scatter(ray: Ray&, rec: HitInfo&, attenuation: Color&): bool",
        "+ emitted(uv: Vector2, p: Vector3): Color",
      )
    ), name: <material>),

    node((0, 0), class(
      "Texture", 
      none, 
      (
        "+ get(rec: HitInfo): Color",
      )
    ), name: <texture>),

    node((1, -0.3), class(
      "ImageTexture", 
      (
        "- image: uint8_t*",
      ), 
      none
    ), name: <imagetexture>),

    node((1, 0.3), class(
      "ColorTexture", 
      (
        "- color: Color",
      ), 
      none
    ), name: <colortexture>),

    node((0, 5), class(
      "Sphere", 
      (
        "- radius: float",
      ), 
      none
    ), name: <sphere>),


    node((-1, 3.5), class(
      "Mesh", 
      (
        "- triangles: vector<Triangle>",
        "bvh_root: unique_ptr<BVHNode>"
      ), 
      none
    ), name: <mesh>),

    node((-1, 4.7), class(
      "Triangle",
      (
        "+ points: Vector3[3]",
      ),
      none
    ), name: <triangle>),

    node((1, 1), class(
      "Color",
      none,
      (
        "+ get_abgr(): uint32_t",
      )
    ), name: <color>),
    node((0.5, 2), class(
      "Vector2",
      (
        "+ x: float",
        "+ y: float"
      ),
      none
    ), name: <vec2>),
    node((1, 2), class(
      "Vector3",
      (
        "+ x: float",
        "+ y: float",
        "+ z: float"
      ),
      none
    ), name: <vec3>),

    node((0.8, 3.5), class(
      "Ray",
      (
        "+ origin: Vector3",
        "+ direction: Vector3"
      ),
      none
    )),
    node((0.8, 5), class(
      "AABB",
      (
        "+ min: Vector3",
        "+ max: Vector3"
      ),
      (
        "+ intersect(ray: Ray&): bool",
      )
    )),

    node((0, 6), class(
      "RNG",
      none,
      (
        "+ get_uniform(from: float, to: float): float",
        "+ get_normal(std: float, dev: float): float",
        "+ get_direction(): Vector3"
      )
    )),

    node((-1, 6), class(
      "HitInfo",
      (
        "+ hit_point: Vector3",
        "+ normal: Vector3",
        "+ uv: Vector2",
        "+ front_face: bool",
        "+ object: Hittable*"
      ),
      none
    )),

    edge(<app>, <raytracer>, marks: composition),

    edge(<raytracer>, <camera>, marks: composition),
    edge(<raytracer>, <scene>, marks: composition),

    edge(<scene>, <hittable>, marks: composition, bend: -8deg),
    edge(<scene>, <material>, marks: composition, bend: 16deg),
    edge(<scene>, <texture>, marks: composition),
    edge(<scene>, <bvhnode>, marks: composition),

    edge(<bvhnode>, <bvhnode>, marks: composition, bend: 110deg, loop-angle: 150deg),

    edge(<mesh>, <triangle>, marks: composition),
    edge(<mesh>, <bvhnode>, marks: composition),

    edge(<bvhnode>, <hittable>, marks: aggregation),
    edge(<hittable>, <material>, marks: aggregation),
    edge(<material>, <texture>, marks: aggregation),

    edge(<sphere>, <hittable>, marks: inheritance),
    edge(<mesh>, <hittable>, marks: inheritance),
    edge(<triangle>, <hittable>, marks: inheritance),

    edge(<imagetexture>, <texture>, marks: inheritance),
    edge(<colortexture>, <texture>, marks: inheritance),

    edge(<color>, <vec3>, marks: inheritance),
    edge(<colortexture>, <color>, marks: composition),
  )
]

#pagebreak()

=== Cây tìm kiếm BVH (Bounding Volume Hierachy)
Thay vì duyệt tuyến tính trên danh sách các vật thể để xem tia hiện tại cắt vật nào gần nhất thì xây dựng cây nhị phân tìm kiếm để duyệt nhanh hơn.

Cách xây:
+ Chia các vật thể thành 2 nhóm có thể tích xấp xỉ nhau, 2 nhóm này sẽ tạo ra 2 nút con trên nút gốc ban đầu.
+ Tếp tục chia như vậy cho đến khi số vật thể trong nhóm không lớn hơn 1 giới hạn nhất định. Khi này nút cuối cùng của 1 nhánh sẽ chứa nhóm vật thể này và là nút lá.

Khi tiến hành dò tia, chỉ cần kiểm tra xem tia có đi qua hộp bao căn theo trục (AABB) của 1 nhóm hay không. Nếu có thì tiếp tục xét 2 nút con của nút này. Cứ như vậy cho đến khi duyệt đến nút lá rồi tiến hành duyệt tuần tự các vật thể trong nhóm.

== Thuật toán
Chương trình sử dụng thuật toán cốt lõi là thuật toán dò tia cơ bản.

==== Đầu vào
- Vị trí của Camera và lưới điểm ảnh trên mặt phẳng chiếu.
- Cấu trúc dữ liệu chứa các vật thể cần kết xuất trong không gian (cây BVH).
- Các thông số môi trường (hàm ánh sáng nền, số lần dội tối đa).

==== Đầu ra
- Ma trận điểm ảnh mang thông tin màu sắc, tạo thành ảnh kết xuất cuối cùng từ góc nhìn của Camera.

==== Phương pháp
Với mỗi điểm ảnh trên màn hình, màu sắc được tính toán thông qua các bước sau:

+ *Khởi tạo tia:* 
  Tạo một tia sáng `ray` có gốc tọa độ tại vị trí Camera, vector hướng đi qua điểm ảnh đang xét. Khởi tạo biến `color = (0, 0, 0)` để tích lũy màu sắc kết quả, và biến `ray_color = (1, 1, 1)` để theo dõi mức độ suy hao năng lượng của tia qua từng lần dội.

+ *Dò tia:* Thực hiện vòng lặp dò tia (tối đa bằng số lần dội cho phép):
  + *Kiểm tra giao cắt:*

    Tìm giao điểm gần nhất của `ray` với các vật thể trong không gian.
    - Nếu tia *không cắt* vật thể nào: Cộng phần ánh sáng môi trường vào kết quả: `color += ray_color * màu nền`. Kết thúc việc theo vết tia này và trả về kết quả dò tia là `color`.
    - Nếu tia *cắt* một vật thể tại điểm giao, tiếp tục các bước sau.
  
  + *Tính toán phát xạ và suy hao:* 
    - Nếu vật thể là nguồn sáng, cộng phần năng lượng phát xạ của nó vào tổng màu: `color = color + ray_color * (màu vật thể * cường độ phát sáng)`.
    - Cập nhật mức độ suy hao của tia sáng khi đập vào bề mặt: `ray_color = ray_color * màu vật thể`.

  + *Sinh tia thứ cấp:*

    Cập nhật điểm gốc của `ray` thành điểm giao vừa tìm được. Dựa vào thuộc tính vật liệu của vật thể, hướng đi mới của `ray` được tính toán như sau:

    *Trường hợp 1: Vật liệu trong suốt có khúc xạ với chiết suất $eta$*

    Tia sáng có thể bị khúc xạ xuyên qua vật thể hoặc phản xạ toàn phần dựa trên định luật Snell. Với $arrow(d)$ là hướng tia tới, $arrow(n)$ là pháp tuyến bề mặt:

    $ eta = n_1 / n_2 $
    $ cos theta = arrow(d) dot arrow(n) $

    Nếu xảy ra phản xạ toàn phần ($sin^2 theta' = eta^2 (1 - cos^2 theta) > 1$) hoặc theo xác suất phản xạ Schlick, hướng tia mới là tia phản xạ:

    $ arrow(d)_"new" = arrow(d) - 2(arrow(d) dot arrow(n))arrow(n) $
    Ngược lại, tia sáng bị khúc xạ xuyên qua vật thể:
    $ arrow(d)_"new" = eta arrow(d) + (eta cos theta - sqrt(1 - eta^2 (1 - cos^2 theta))) arrow(n) $

    *Trường hợp 2: Vật liệu chắn sáng với độ nhám $r$*

    Hướng tia mới $arrow(d)_"new"$ được nội suy tuyến tính giữa hướng phản xạ gương $arrow(d)_"spec"$ và hướng tán xạ khuếch tán ngẫu nhiên $arrow(d)_"diff"$:

    $ arrow(d)_"spec" = arrow(d) - 2(arrow(d) dot arrow(n))arrow(n) $
    $ arrow(d)_"diff" = (arrow(n) + arrow(v)_"rand") / arrow(n) + arrow(v)_"rand" $
    $ arrow(d)_"new" = (1 - r) arrow(d)_"spec" + r arrow(d)_"diff" $

+ *Lặp lại Bước 2*: với tia `ray` vừa được cập nhật hướng và gốc mới, quá trình tiếp tục cho đến khi tia bay ra ngoài không gian hoặc đạt giới hạn số lần dội. Màu `color` cuối cùng sẽ là màu của điểm ảnh.
