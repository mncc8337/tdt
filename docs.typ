#import "@preview/fletcher:0.5.8" as fletcher: diagram, node, edge

#let class(name, attributes, methods, size: 7pt) = block(
  stroke: 0.5pt,
  radius: 2pt,
  inset: 0pt,
  width: auto,
  [
    #set block(spacing: 2pt)
    #pad(5pt)[#align(center)[#text(font: "CaskaydiaCove NF")[*#name*]]]
    #if attributes != none [
      #line(length: 100%, stroke: 0.5pt)
      #pad(5pt)[#align(left)[#text(size: size, font: "CaskaydiaCove NF")[#attributes.join("\n")]]]
    ]
    #if methods != none [
      #line(length: 100%, stroke: 0.5pt)
      #pad(5pt)[#align(left)[#text(size: size, font: "CaskaydiaCove NF")[#methods.join("\n")]]]
    ]
  ]
)
#let class-sm = class.with(size: 6.5pt)
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

#set heading(numbering: "1.1.")

#show heading.where(level: 1): it => {
  counter(figure).update(0)
  it
}

#set figure(supplement: [Hình])
#set figure(numbering: n => numbering("1.1", counter(heading).get().first(), n))
#show figure.caption: it => align(center)[
  #text(size: 9pt, style: "italic")[
    #it.supplement #counter(figure).display(it.numbering). #it.body
  ]
]

#show raw: set text(font: "CaskaydiaCove NF")

#align(center)[#text(size: 24pt, weight: "bold")[Trình dò tia trên CPU]]
#v(1em)
= Mục tiêu
- Vận dụng kiến thức Lập trình hướng đối tượng để thiết kế kiến trúc phần mềm linh hoạt, có tính đóng gói cao và dễ dàng mở rộng các loại vật thể, vật liệu mới.
- Ứng dụng Cấu trúc dữ liệu (cụ thể là cây nhị phân tìm kiếm) để giải quyết bài toán tối ưu hóa hiệu năng, giảm thiểu thời gian tính toán giao cắt trong không gian.
- Xây dựng thành công một trình dò tia trên CPU có khả năng mô phỏng các định luật quang học và kết xuất hình ảnh chân thực.

= Nhiệm vụ
== Về tính toán
- Cài đặt hệ thống hình học toán học cơ bản (Vector, Tia, Hộp bao AABB, Hình cầu, Tam giác).
- Xử lý tính toán giao cắt tia với các hình học cơ bản (mặt cầu) và phức tạp (lưới đa giác tạo từ nhiều tam giác).
- Mô phỏng các tính chất vật liệu quang học: phản xạ gương (kim loại), khúc xạ (thủy tinh/nước) và tán xạ khuếch tán (bề mặt nhám).
- Tính toán ánh sáng toàn cục bằng thuật toán Path Tracing.

== Về cấu trúc dữ liệu
- Cài đặt cấu trúc dữ liệu cây BVH (Bounding Volume Hierarchy) để tăng tốc độ truy vấn giao điểm giữa tia và hàng ngàn vật thể trong không gian.
- Ứng dụng đa luồng để phân chia công việc tính toán các điểm ảnh lên nhiều lõi CPU.

== Về giao diện
- Xây dựng không gian làm việc trực quan.
- Xây dựng các chức năng tương tác thời gian thực:
  - Thay đổi thuộc tính vật thể (vị trí, góc quay, màu sắc, chiết suất, độ nhám, cường độ sáng).
  - Điều hướng Camera (vị trí, góc nhìn, tiêu cự).
- Cung cấp tính năng lưu/tải cảnh và xuất kết quả kết xuất ra tệp tin ảnh PNG/JPG.

= Thư viện sử dụng
- Thư viện đồ họa SFML kết hợp với thư viện giao diện người dùng ImGUI
- Thư viện đa luồng OpenMP được cài đặt sẵn trong gcc/clang/msvc
- Thư viện mảng động `std::vector` có sẵn của C++
- Thư viện sinh số ngẫu nhiên `std::random` có sẵn của C++11
- Thư viện quản lý bộ nhớ thông minh `std::memory` có sẵn của C++11

#pagebreak()

= Cấu trúc dữ liệu
== Tổng quan các lớp

Các sơ đồ lớp dưới đây được xây dựng trực tiếp từ cấu trúc của các tệp tiêu đề (`.h`) trong thư mục `src/`. Hệ thống được chia thành bốn nhóm chính: kiến trúc ứng dụng và cảnh, hệ thống hình học, hệ thống vật liệu – kết cấu – môi trường, và các kiểu toán học hỗ trợ.

#text(size: 9pt)[
  *Ký hiệu:* ◆ thành phần (composition) – đối tượng sở hữu; ◇ tập hợp (aggregation) – đối tượng tham chiếu; ▷ kế thừa (inheritance); ⇢ phụ thuộc (dependency). Một số chữ ký phương thức dài được viết gọn bằng `...`; chi tiết đầy đủ có trong các tệp tiêu đề.
]

#figure(
  block(breakable: false, width: 100%)[
  #align(center)[
    #diagram(
      node-shape: rect,
      spacing: (0.2cm, 0.2cm),

      node((-1, 0), class(
        "Gui",
        (
          "- viewport_width, viewport_height: unsigned",
          "- camera: Camera&",
          "- scene: Scene&",
          "- tracer: RayTracer",
          "- selected_*: Hittable*/Material*/Texture*/Medium*",
          "- window: sf::RenderWindow",
          "- render_thread: thread",
          "- scene_mutex: mutex",
        ),
        (
          "+ Gui(camera, scene, width, height)",
          "+ ~Gui()",
          "+ run()",
          "- renderRoutine()",
          "- handlePicking()",
          "- postEdit(edit)",
          "- applyPendingEdits()",
          "- draw*Window()",
          "- draw*PropertiesSection()",
        )
      ), name: <gui>),

      node((0, 0), class(
        "RayTracer",
        (
          "- viewport_width, viewport_height: unsigned",
          "- max_bounces: unsigned = 50",
          "- buffer: vector<Color>",
          "- pixels: vector<uint32_t>",
        ),
        (
          "+ RayTracer(width, height)",
          "+ getData(): const uint8_t*",
          "+ trace(ray: Ray, scene: Scene&): Color",
          "+ reset()",
          "+ render(camera: Camera&, scene: Scene&, pass: unsigned)",
        )
      ), name: <raytracer>),

      node((-1, 1.3), class(
        "Camera",
        (
          "- focal_length, exposure: float",
          "- aperture, diverge_strength: float",
          "- position, lookat: Vec3",
          "- w, u, v: Vec3",
        ),
        (
          "+ Camera(focal_length, position, lookat)",
          "+ getRayAt(uv: Vec2, jitter: bool): Ray",
          "+ lookAt(v: Vec3&)",
          "+ get/setPosition()",
          "+ get/setDirection()",
          "- computeOrientation()",
        )
      ), name: <camera>),

      node((0, 1.3), class(
        "Scene",
        (
          "- objects: vector<unique_ptr<Hittable>>",
          "- materials: vector<unique_ptr<Material>>",
          "- textures: vector<unique_ptr<Texture>>",
          "- mediums: vector<unique_ptr<Medium>>",
          "- global_medium: Medium*",
          "- flat_bvh: vector<LinearBVHNode>",
        ),
        (
          "+ addObject(obj): Hittable&",
          "+ addMaterial(mat): Material&",
          "+ addTexture(tex): Texture&",
          "+ addMedium(med): Medium&",
          "+ getGlobalMedium(): Medium*&",
          "+ getClosest(ray, obj): HitInfo",
          "+ buildBVH()",
          "- flattenBVH(node, offset): int",
        )
      ), name: <scene>),

      node((-1, 3), class(
        "BVHNode",
        (
          "+ aabb: AABB",
          "+ left, right: BVHNode*",
          "+ leaf_node: bool",
          "+ start_index, num_objects, axis: int",
        ),
        (
          "+ BVHNode(objects, start, end)",
          "+ ~BVHNode()",
          "+ {static} build(flat_bvh, objects, n)",
          "+ {static} flatten(flat_bvh, node, offset): int",
          "+ {static} traverse(flat_bvh, ray, hitFunc): HitInfo",
        )
      ), name: <bvhnode>),

      node((0, 3), class(
        "LinearBVHNode",
        (
          "+ aabb: AABB",
          "+ primitives_offset / right_offset: int",
          "+ num_objects: uint16_t",
          "+ axis: uint8_t",
        ),
        none
      ), name: <linearbvh>),

      edge(<gui>, <raytracer>, marks: composition),
      edge(<gui>, <camera>, marks: aggregation),
      edge(<gui>, <scene>, marks: aggregation, bend: -12deg),
      edge(<raytracer>, <camera>, marks: "-->", bend: 10deg),
      edge(<raytracer>, <scene>, marks: "-->"),

      edge(<scene>, <bvhnode>, marks: "-->", bend: -10deg),
      edge(<scene>, <linearbvh>, marks: composition),
      edge(<bvhnode>, <linearbvh>, marks: "-->"),
      edge(<bvhnode>, <bvhnode>, marks: composition, bend: 110deg, loop-angle: 150deg),
    )
  ]
  ], caption: [Kiến trúc ứng dụng và cảnh.])

#figure(
  block(breakable: false, width: 100%)[
  #align(center)[
    #diagram(
      node-shape: rect,
      spacing: (0.2cm, 0.2cm),

      node((-1, 0), class(
        "Transform",
        (
          "+ scale: Vec3",
          "+ rotation: Mat3x3",
          "+ rotation_angles: Vec3",
          "+ translation: Vec3",
        ),
        (
          "+ pointApply(v: Vec3&): Vec3",
          "+ pointApplyInverse(v: Vec3&): Vec3",
          "+ dirApply(v: Vec3&): Vec3",
          "+ dirApplyInverse(v: Vec3&): Vec3",
          "+ normalApply(v: Vec3&): Vec3",
          "+ rotate(angles: Vec3): Transform&",
          "+ move(pos: Vec3): Transform&",
          "+ size(s: Vec3): Transform&",
        )
      ), name: <transform>),

      node((0, 0), class(
        "Hittable «abstract»",
        (
          "# transform: Transform",
          "# material: Material*",
          "# name: string",
        ),
        (
          "+ hit(ray: Ray&): HitInfo = 0",
          "+ getAABB(): AABB",
          "# getLocalAABB(): AABB = 0",
          "+ getTransform(): Transform&",
          "+ getMaterial(): Material*",
          "+ getName(): string&",
        )
      ), name: <hittable>),

      node((-1, 2), class(
        "Sphere",
        (
          "- center: Vec3",
          "- radius: float",
        ),
        (
          "+ Sphere(name, material, center, radius)",
          "+ hit(ray: Ray&): HitInfo",
          "+ getRadius(): float&",
          "# getLocalAABB(): AABB",
        )
      ), name: <sphere>),

      node((0, 2), class(
        "Mesh",
        (
          "- tris: vector<RawTriangle>",
          "- flat_bvh: vector<LinearBVHNode>",
          "- aabb: AABB",
        ),
        (
          "+ Mesh(name, material, tris)",
          "+ Mesh(name, material, filename)",
          "+ hit(ray: Ray&): HitInfo",
          "# getLocalAABB(): AABB",
        )
      ), name: <mesh>),

      node((1, 2), class(
        "Triangle",
        (
          "- triangle: RawTriangle",
        ),
        (
          "+ Triangle(name, material, v0, v1, v2)",
          "+ hit(ray: Ray&): HitInfo",
          "+ getAABB(): AABB",
          "# getLocalAABB(): AABB",
        )
      ), name: <triangle>),

      node((0, 3.5), class(
        "RawTriangle",
        (
          "+ v0, v1, v2: Vec3",
          "+ uv0, uv1, uv2: Vec2",
        ),
        (
          "+ RawTriangle(v0, v1, v2)",
          "+ RawTriangle(v0, v1, v2, uv0, uv1, uv2)",
          "+ hit(ray, t_min, t_max, tuv): bool",
        )
      ), name: <rawtriangle>),

      node((1, 3.5), class(
        "LinearBVHNode",
        (
          "+ aabb: AABB",
          "+ primitives_offset / right_offset: int",
          "+ num_objects: uint16_t",
          "+ axis: uint8_t",
        ),
        none
      ), name: <linearbvh>),

      edge(<transform>, <hittable>, marks: composition),
      edge(<sphere>, <hittable>, marks: inheritance),
      edge(<mesh>, <hittable>, marks: inheritance),
      edge(<triangle>, <hittable>, marks: inheritance),
      edge(<rawtriangle>, <mesh>, marks: composition),
      edge(<rawtriangle>, <triangle>, marks: composition),
      edge(<mesh>, <linearbvh>, marks: composition, bend: -12deg),
    )
  ]
  ], caption: [Hệ thống hình học.])

Hệ thống hình học được xây dựng quanh lớp cơ sở trừu tượng `Hittable`. Mỗi vật thể lưu một `Transform` (gồm tỉ lệ, ma trận quay `Mat3x3`, góc quay và vị trí) cùng con trỏ tới `Material`. Lớp này khai báo hai phương thức thuần ảo `hit()` và `getLocalAABB()`; các lớp dẫn xuất chỉ cần cài đặt phép giao cắt trong không gian cục bộ, còn `getAABB()` sẽ áp dụng `Transform` để trả về hộp bao trong không gian thế giới.

- `Sphere` lưu tâm và bán kính, phù hợp cho các vật thể tròn cơ bản.
- `Triangle` bọc một `RawTriangle`, là đơn vị hình học nhỏ nhất của lưới.
- `Mesh` quản lý một danh sách `RawTriangle` và một BVH phẳng (`LinearBVHNode`) để tăng tốc truy vấn giao cắt trên lưới lớn.
- `RawTriangle` cài đặt thuật toán Moller–Trumbore để kiểm tra giao cắt tia–tam giác và nội suy tọa độ `uv`.

#figure(
  block(breakable: false, width: 100%)[
  #align(center)[
    #diagram(
      node-shape: rect,
      spacing: (0.2cm, 0.2cm),

      node((-1, 0), class(
        "Matte",
        none,
        (
          "+ Matte(name, texture)",
          "+ scatter(...): ScatterResult",
        )
      ), name: <matte>),

      node((0, 0), class(
        "Metal",
        (
          "- roughness: float",
        ),
        (
          "+ Metal(name, texture, roughness)",
          "+ scatter(...): ScatterResult",
          "+ getRoughness(): float&",
        )
      ), name: <metal>),

      node((1, 0), class(
        "Dielectric",
        none,
        (
          "+ Dielectric(name, texture, medium)",
          "+ scatter(...): ScatterResult",
        )
      ), name: <dielectric>),

      node((1.5, 1.4), class(
        "Light",
        (
          "- emission_strength: float",
          "- color: Color",
        ),
        (
          "+ Light(name, texture, strength, color)",
          "+ emitted(rec): Color",
          "+ getEmissionStrength(): float&",
          "+ getColor(): Color&",
        )
      ), name: <light>),

      node((0, 2.5), class(
        "Material",
        (
          "# name: string",
          "# texture: Texture*",
          "# medium: Medium*",
        ),
        (
          "+ scatter(...): ScatterResult",
          "+ emitted(rec): Color",
          "+ getName(): string&",
          "+ getTexture(): Texture*",
          "+ getMedium(): Medium*",
        )
      ), name: <material>),

      node((0, 3.9), class(
        "ScatterResult «enum»",
        (
          "Terminated",
          "Scattered",
          "Refracted",
        ),
        none
      ), name: <scatterresult>),

      edge(<matte>, <material>, marks: inheritance),
      edge(<metal>, <material>, marks: inheritance),
      edge(<dielectric>, <material>, marks: inheritance),
      edge(<light>, <material>, marks: inheritance),
      edge(<material>, <scatterresult>, marks: "-->"),
    )
  ]
  ], caption: [Hệ thống vật liệu.])

#figure(
  block(breakable: false, width: 100%)[
  #align(center)[
    #diagram(
      node-shape: rect,
      spacing: (0.2cm, 0.2cm),

      node((-1, 0), class(
        "Texture «abstract»",
        (
          "# name: string",
        ),
        (
          "+ get(rec: HitInfo&): Color = 0",
          "+ getName(): string&",
        )
      ), name: <texture>),

      node((0, 0), class(
        "Material",
        (
          "# name: string",
          "# texture: Texture*",
          "# medium: Medium*",
        ),
        (
          "+ scatter(...): ScatterResult",
          "+ emitted(rec): Color",
          "+ getName(): string&",
          "+ getTexture(): Texture*",
          "+ getMedium(): Medium*",
        )
      ), name: <material2>),

      node((1, 0), class(
        "Medium",
        (
          "- name: string",
          "- ior: float",
        ),
        (
          "+ transmit(distance: float): Color",
          "+ getName(): string&",
          "+ getIOR(): float&",
        )
      ), name: <medium>),

      node((-1, 1.6), class(
        "ColorTexture",
        (
          "- color: Color",
        ),
        (
          "+ ColorTexture(name, color)",
          "+ get(rec: HitInfo&): Color",
          "+ getColor(): Color&",
        )
      ), name: <colortexture>),

      node((0, 1.6), class(
        "HitInfo",
        (
          "+ distance: float",
          "+ hit_point: Vec3",
          "+ normal: Vec3",
          "+ material: const Material*",
          "+ front_face: bool",
          "+ uv: Vec2",
        ),
        none
      ), name: <hitinfo>),

      node((1, 1.6), class(
        "Homogeneous",
        (
          "- sigma: Color",
          "- density: float",
        ),
        (
          "+ Homogeneous(name, ior, sigma, density)",
          "+ transmit(distance: float): Color",
          "+ getSigma(): Color&",
          "+ getDensity(): float&",
        )
      ), name: <homogeneous>),

      edge(<material2>, <texture>, marks: aggregation),
      edge(<material2>, <medium>, marks: aggregation),
      edge(<colortexture>, <texture>, marks: inheritance),
      edge(<homogeneous>, <medium>, marks: inheritance),
      edge(<hitinfo>, <material2>, marks: aggregation),
    )
  ]
  ], caption: [Kết cấu, vật liệu và môi trường truyền dẫn.])

#figure(
  block(breakable: false, width: 100%)[
  #align(center)[
    #diagram(
      node-shape: rect,
      spacing: (0.2cm, 0.2cm),

      node((-1, 0), class-sm(
        "Vec2",
        (
          "+ x, y: float",
        ),
        (
          "+ Vec2(t: float)",
          "+ Vec2(x, y)",
          "+ Vec2(v: Vec3&)",
          "+ operator[](axis): float&",
          "+ operator+(v), operator-(v)",
          "+ operator*(t), operator/(t): Vec2",
          "+ dot(other): float",
          "+ length(), length_squared(): float",
          "+ normalized(): Vec2",
          "+ normalize(): Vec2",
          "+ lerp(v, t): Vec2",
        )
      ), name: <vec2>),

      node((0, 0), class-sm(
        "Vec3",
        (
          "+ x, y, z: float",
        ),
        (
          "+ Vec3(t: float)",
          "+ Vec3(x, y, z)",
          "+ operator[](axis): float&",
          "+ operator+(v), operator-(v)",
          "+ operator*(t), operator*(v)",
          "+ operator/(t), operator/(v): Vec3",
          "+ dot(other), cross(other): float/Vec3",
          "+ length(), length_squared(): float",
          "+ normalized(): Vec3",
          "+ normalize(): Vec3",
          "+ lerp(v, t): Vec3",
          "+ reflection(n): Vec3",
          "+ refraction(n, eta): Vec3",
          "+ toABGR(): uint32_t",
          "+ clampColor(): Vec3&",
        )
      ), name: <vec3>),

      node((1, 0), class-sm(
        "Color «typedef»",
        (
          "Color = Vec3",
        ),
        none
      ), name: <color>),

      node((-1, 2.8), class-sm(
        "Ray",
        (
          "+ origin: Vec3",
          "+ direction: Vec3",
        ),
        (
          "+ Ray(origin, direction)",
          "+ point(distance: float): Vec3",
        )
      ), name: <ray>),

      node((0, 2.8), class-sm(
        "AABB",
        (
          "+ minp, maxp: Vec3",
          "+ centroid: Vec3",
        ),
        (
          "+ AABB(p: Vec3)",
          "+ AABB(points: Vec3[])",
          "+ extend(p: Vec3&)",
          "+ extend(aabb: AABB&)",
          "+ hit(ray: Ray&, tmin, tmax): bool",
          "+ operator+(p: Vec3&): AABB",
          "+ operator+(aabb: AABB&): AABB",
        )
      ), name: <aabb>),

      node((1, 2.8), class-sm(
        "RawTriangle",
        (
          "+ v0, v1, v2: Vec3",
          "+ uv0, uv1, uv2: Vec2",
        ),
        (
          "+ RawTriangle(v0, v1, v2)",
          "+ RawTriangle(v0, v1, v2, uv0, uv1, uv2)",
          "+ hit(ray, t_min, t_max, tuv): bool",
        )
      ), name: <rawtriangle>),

      node((-1, 5.4), class-sm(
        "Transform",
        (
          "+ scale: Vec3",
          "+ rotation: Mat3x3",
          "+ rotation_angles: Vec3",
          "+ translation: Vec3",
        ),
        (
          "+ pointApply(v): Vec3",
          "+ pointApplyInverse(v): Vec3",
          "+ dirApply(v): Vec3",
          "+ dirApplyInverse(v): Vec3",
          "+ normalApply(v): Vec3",
          "+ rotate(angles: Vec3): Transform&",
          "+ move(pos: Vec3): Transform&",
          "+ size(s: Vec3): Transform&",
        )
      ), name: <transform>),

      node((0, 5.4), class-sm(
        "Mat3x3",
        (
          "+ m[3][3]: float",
        ),
        (
          "+ Mat3x3()",
          "+ operator*(v: Vec3&): Vec3",
          "+ operator*(right: Mat3x3&): Mat3x3",
          "+ transpose(): Mat3x3",
          "+ {static} rotateX(angle): Mat3x3",
          "+ {static} rotateY(angle): Mat3x3",
          "+ {static} rotateZ(angle): Mat3x3",
        )
      ), name: <mat3x3>),

      node((1, 5.4), class-sm(
        "RNG",
        (
          "- generator: mt19937",
          "- uniform_dist: uniform_real_distribution",
          "- normal_dist: normal_distribution",
        ),
        (
          "+ {static} uniform(): float",
          "+ {static} uniform(from, to): float",
          "+ {static} normal(): float",
          "+ {static} direction(): Vec3",
          "+ {static} directionFast(): Vec3",
          "+ {static} pointInCircle(radius): Vec2",
          "+ {static} color(): Color",
        )
      ), name: <rng>),

      edge(<vec2>, <vec3>, marks: "-->"),
      edge(<color>, <vec3>, marks: "-->", label: [alias], bend: -10deg),
      edge(<ray>, <vec3>, marks: composition),
      edge(<aabb>, <vec3>, marks: composition),
      edge(<rawtriangle>, <vec3>, marks: composition),
      edge(<transform>, <mat3x3>, marks: composition),
    )
  ]
  ], caption: [Các kiểu toán học hỗ trợ.])

Các kiểu toán học được thiết kế dưới dạng `struct` nhỏ, thao tác trên giá trị và không sở hữu tài nguyên. `Vec3` kiêm luôn vai trò `Color` (typedef) và cung cấp các phép toán vector, phản xạ, khúc xạ, chuyển đổi màu sang ABGR. `Ray` và `AABB` là hai kiểu nền tảng cho mọi phép truy vấn giao cắt; `RawTriangle` kết hợp `Vec3` và `Vec2` để lưu hình học và tọa độ kết cấu. `Mat3x3` và `Transform` đóng gói phép biến đổi affine, còn `RNG` là lớp tiện ích tĩnh phục vụ lấy mẫu ngẫu nhiên trong path tracing.

#pagebreak()

== Cây BVH (Bounding Volume Hierarchy)
Thay vì duyệt tuyến tính qua toàn bộ danh sách vật thể để tìm giao điểm, hệ thống sử dụng cấu trúc cây nhị phân Bounding Volume Hierarchy (BVH) nhằm tối ưu hóa độ phức tạp thời gian truy vấn từ $O(N)$ xuống trung bình $O(log N)$.

=== Thuật toán xây dựng cây
Được thực hiện đệ quy theo phương pháp chia để trị (Divide and Conquer):
+ Xác định hộp bao (AABB - Axis-Aligned Bounding Box) bao trọn toàn bộ các vật thể hiện có.
+ Tìm trục không gian dài nhất của hộp bao (X, Y hoặc Z) để làm trục phân chia.
+ Sắp xếp các vật thể dọc theo trục này và chia thành hai nhóm (nửa trái và nửa phải).
+ Tạo hai nút con (BVHNode) cho hai nhóm vừa chia. Tiếp tục đệ quy quá trình này cho đến khi số lượng vật thể trong một nút giảm xuống dưới một ngưỡng cho trước. Nút đó sẽ trở thành nút lá chứa mảng các vật thể thực tế.

=== Thuật toán truy vấn giao cắt
Khi một tia được bắn ra, chương trình chỉ cần kiểm tra xem tia đó có cắt qua hộp bao AABB của nút gốc hay không:
- Nếu *không cắt*: Bỏ qua hoàn toàn việc kiểm tra nhánh cây đó, giúp tiết kiệm lượng lớn phép tính.
- Nếu *có cắt*: Tiếp tục đệ quy kiểm tra xuống hai nút con. Quá trình lặp lại cho tới khi chạm đến nút lá, lúc này thuật toán mới tiến hành kiểm tra giao cắt chi tiết với từng hình học (Sphere, Triangle, Mesh) nằm trong nút lá đó.

= Thuật toán
== Thuật toán dò tia cơ bản.
=== Đầu vào
- Vị trí của Camera và lưới điểm ảnh trên mặt phẳng chiếu.
- Cấu trúc dữ liệu chứa các vật thể cần kết xuất trong không gian (cây BVH).
- Các thông số môi trường (hàm ánh sáng nền, số lần dội tối đa).

=== Đầu ra
- Ma trận điểm ảnh mang thông tin màu sắc, tạo thành ảnh kết xuất cuối cùng từ góc nhìn của Camera.

=== Phương pháp
Với mỗi điểm ảnh trên màn hình, màu sắc được tính toán thông qua các bước sau:

+ *Khởi tạo tia:* 
  Tạo một tia sáng `ray` có gốc tọa độ tại vị trí Camera, vector hướng đi qua điểm ảnh đang xét. Khởi tạo biến `color = (0, 0, 0)` để tích lũy màu sắc kết quả, và biến `attenuation = (1, 1, 1)` để theo dõi mức độ suy hao năng lượng của tia qua từng lần dội.

+ *Dò tia:* Thực hiện vòng lặp dò tia (tối đa bằng số lần dội cho phép):
  + *Kiểm tra giao cắt:*

    Tìm giao điểm gần nhất của `ray` với các vật thể trong không gian.
    - Nếu tia *không cắt* vật thể nào: Cộng phần ánh sáng môi trường vào kết quả: `color += attenuation * màu nền`. Kết thúc việc theo vết tia này và trả về kết quả dò tia là `color`.
    - Nếu tia *cắt* một vật thể tại điểm giao, tiếp tục các bước sau.
  
  + *Tính toán phát xạ và suy hao:* 
    - Nếu vật thể là nguồn sáng, cộng phần năng lượng phát xạ của nó vào tổng màu: `color += attenuation * (màu vật thể * cường độ phát sáng)`.
    - Cập nhật mức độ suy hao của tia sáng khi đập vào bề mặt: `attenuation *= màu vật thể`.

  + *Sinh tia thứ cấp:*

    Cập nhật điểm gốc của `ray` thành điểm giao vừa tìm được. Dựa vào thuộc tính vật liệu của vật thể, hướng đi mới của `ray` được tính toán như sau:

    *Trường hợp 1: Vật liệu trong suốt có khúc xạ với chiết suất $eta$*

    Tia sáng có thể bị khúc xạ xuyên qua vật thể hoặc phản xạ toàn phần dựa trên định luật Snell. Với $arrow(d)$ là hướng tia tới và $arrow(n)$ là pháp tuyến bề mặt hướng về phía tia tới:

    $ eta = n_1 / n_2 $
    $ cos theta = -arrow(d) dot arrow(n) $

    Nếu xảy ra phản xạ toàn phần ($sin^2 theta' = eta^2 (1 - cos^2 theta) > 1$) hoặc theo xác suất phản xạ Schlick, hướng tia mới là tia phản xạ:

    $ arrow(d)_"new" = arrow(d) - 2(arrow(d) dot arrow(n))arrow(n) $
    Ngược lại, tia sáng bị khúc xạ xuyên qua vật thể:
    $ arrow(d)_"new" = eta arrow(d) + (eta cos theta - sqrt(1 - eta^2 (1 - cos^2 theta))) arrow(n) $

    *Trường hợp 2: Vật liệu chắn sáng*

    Với vật liệu tán xạ khuếch tán (`Matte`), hướng tia mới được lấy ngẫu nhiên trong bán cầu quanh pháp tuyến:

    $ arrow(d)_"diff" = "normalize"(arrow(n) + arrow(v)_"rand") $

    Nếu $arrow(n) + arrow(v)_"rand"$ gần vector không, hướng tán xạ được gán bằng $arrow(n)$ để tránh chia cho 0.

    Với vật liệu kim loại (`Metal`), hướng phản xạ gương được tính bởi:

    $ arrow(d)_"spec" = arrow(d) - 2(arrow(d) dot arrow(n))arrow(n) $

    Khi độ nhám $r > 0$, một vector ngẫu nhiên $arrow(v)_"rand"$ có độ lớn tỉ lệ với $r$ được cộng vào hướng phản xạ:

    $ arrow(d)_"new" = "normalize"(arrow(d)_"spec" + r arrow(v)_"rand") $

    Nếu $arrow(d)_"new" dot arrow(n) <= 0$, hướng mới quay ra sau bề mặt; khi đó cài đặt phản xạ lại vector này qua $arrow(n)$ trước khi chuẩn hóa để tia tiếp tục đi vào nửa không gian phía trên bề mặt.

+ *Lặp lại Bước 2*: với tia `ray` vừa được cập nhật hướng và gốc mới, quá trình tiếp tục cho đến khi tia bay ra ngoài không gian hoặc đạt giới hạn số lần dội. Màu `color` cuối cùng sẽ là màu của điểm ảnh.

== Thuật toán giao cắt tia – mặt cầu

=== Đầu vào
- Tia `ray` trong không gian thế giới, gồm gốc `origin` và hướng `direction`.
- Mặt cầu `Sphere` với tâm `center`, bán kính `radius` và phép biến đổi `transform`.

=== Đầu ra
- Một cấu trúc `HitInfo` chứa khoảng cách `distance`, điểm giao `hit_point`, pháp tuyến `normal`, cờ `front_face` và vật liệu của mặt cầu.
- Nếu tia không cắt mặt cầu, `distance` giữ giá trị mặc định `-1` và macro `DID_HIT` trả về `false`.

=== Phương pháp
Để hỗ trợ dịch, quay và co giãn vật thể, phép kiểm tra được thực hiện trong không gian cục bộ của mặt cầu. Gốc và hướng của tia lần lượt được đưa về không gian này bằng nghịch đảo của `Transform`:

$ arrow(o)' = T^(-1)(arrow(o)), quad arrow(d)' = T^(-1)(arrow(d)) $

Đặt $arrow(m) = arrow(o)' - arrow(c)$, với $arrow(c)$ là tâm mặt cầu. Điều kiện để một điểm trên tia nằm trên mặt cầu là:

$ ||arrow(o)' + t arrow(d)' - arrow(c)||^2 = r^2 $

Khai triển và thu gọn phương trình bậc hai theo $t$:

$ (arrow(d)' dot arrow(d)')t^2 + 2(arrow(m) dot arrow(d)')t + (arrow(m) dot arrow(m) - r^2) = 0 $

Chương trình sử dụng dạng rút gọn với nửa hệ số bậc nhất để giảm số phép nhân:

$ a = arrow(d)' dot arrow(d)', quad b_"half" = -(arrow(m) dot arrow(d)'), quad c = arrow(m) dot arrow(m) - r^2 $

$ Delta = b_"half"^2 - a c $

Nếu $Delta < 0$, tia không cắt mặt cầu. Ngược lại, hai nghiệm là:

$ t_0 = (b_"half" - sqrt(Delta))/a, quad t_1 = (b_"half" + sqrt(Delta))/a $

Chương trình ưu tiên nghiệm nhỏ hơn, $t_0$. Nếu $t_0 < epsilon$, nghĩa là giao điểm nằm phía sau tia hoặc quá gần gốc tia, nghiệm $t_1$ được chọn thay thế. Nếu cả hai nghiệm đều nhỏ hơn $epsilon$, phép kiểm tra thất bại. Bước này vừa tránh hiện tượng tự cắt (shadow acne), vừa xử lý trường hợp gốc tia nằm bên trong mặt cầu.

Sau khi có khoảng cách $t$, điểm giao và pháp tuyến ngoài trong không gian cục bộ được tính bởi:

$ arrow(p)' = arrow(o)' + t arrow(d)', quad arrow(n)_"out" = (arrow(p)' - arrow(c))/r $

Cờ `front_face` cho biết tia đang đi vào mặt trước hay mặt sau của mặt cầu:

$ "front_face" = arrow(d)' dot arrow(n)_"out" < 0 $

Nếu `front_face` đúng, pháp tuyến được giữ nguyên; ngược lại, pháp tuyến được đảo dấu để luôn hướng về phía tia tới. Cuối cùng, điểm giao và pháp tuyến được đưa trở lại không gian thế giới:

$ arrow(p) = T(arrow(p)'), quad arrow(n) = N(arrow(n)') $

Khoảng cách cuối cùng được tính lại bằng hình chiếu của vector từ gốc tia đến điểm giao lên hướng tia thế giới:

$ t = (arrow(p) - arrow(o)) dot arrow(d) $

Cách tính lại này cần thiết vì `Transform` có thể chứa phép co giãn, khiến khoảng cách trong không gian cục bộ không còn trùng với khoảng cách thế giới.

Hộp bao cục bộ của mặt cầu được xây dựng đơn giản từ tâm và bán kính:

$ "AABB"_"local" = [arrow(c) - (r, r, r), arrow(c) + (r, r, r)] $

Sau đó `Hittable::getAABB()` biến đổi 8 đỉnh của hộp bao cục bộ để thu được AABB trong không gian thế giới.

Độ phức tạp của phép kiểm tra là $O(1)$.

== Thuật toán giao cắt tia – hộp bao AABB

=== Đầu vào
- Tia `ray` với gốc `origin` và hướng `direction`.
- Hộp bao `AABB` được biểu diễn bởi hai điểm đối diện `minp` và `maxp`.
- Khoảng tham số hợp lệ $[t_min, t_max]$. Trong quá trình duyệt BVH, $t_min$ thường bằng `EPSILON` và $t_max$ bằng khoảng cách tới va chạm gần nhất hoặc `FAR_DISTANCE`.

=== Đầu ra
- `true` nếu tia cắt hộp bao trong khoảng $[t_min, t_max]$.
- `false` nếu tia không cắt hộp bao, cắt ở phía sau gốc tia, hoặc chỉ cắt ngoài khoảng tham số cho phép.

=== Phương pháp
AABB là giao của ba “slab” song song với các mặt phẳng tọa độ $x$, $y$, $z$. Với mỗi trục $a$, giao điểm của tia với hai mặt phẳng $a = "minp"_a$ và $a = "maxp"_a$ là:

$ t_0 = ("minp"_a - o_a)/d_a, quad t_1 = ("maxp"_a - o_a)/d_a $

Nếu $d_a < 0$, thứ tự hai nghiệm bị đảo nên chương trình hoán đổi $t_0$ và $t_1$. Khoảng giao của tia với slab thứ $a$ là $[t_0, t_1]$. Khoảng giao tổng của tia với cả ba slab là giao của ba khoảng:

$ t_"enter" = max(t_"enter", t_0), quad t_"exit" = min(t_"exit", t_1) $

Ban đầu $t_"enter" = t_min$ và $t_"exit" = t_max$. Sau mỗi trục, nếu $t_"exit" < t_"enter"$ thì khoảng giao rỗng và tia không cắt hộp bao. Nếu duyệt hết cả ba trục mà điều kiện vẫn đúng, tia cắt hộp bao.

Về mặt hình học, $t_"enter"$ là tham số mà tia bắt đầu đi vào hộp, còn $t_"exit"$ là tham số mà tia đi ra khỏi hộp. Nếu $t_"exit" < t_"enter"$, tia đi qua các slab theo thứ tự không hợp lệ, tức là không có đoạn nào nằm đồng thời trong cả ba slab.

Chương trình dùng nghịch đảo hướng tia `invD = 1/d[a]` để tránh chia lặp lại cho từng trục:

```cpp
for (int a = 0; a < 3; ++a) {
    float invD = 1.0f / ray.direction[a];
    float t0 = (minp[a] - ray.origin[a]) * invD;
    float t1 = (maxp[a] - ray.origin[a]) * invD;

    if (invD < 0.0f) std::swap(t0, t1);

    tmin = std::max(t0, tmin);
    tmax = std::min(t1, tmax);

    if (tmax < tmin) return false;
}
return true;
```

Trường hợp $d_a = 0$ được xử lý dựa trên số học dấu phẩy động của IEEE 754: `invD` là $plus.minus infinity$, nên $t_0$ và $t_1$ trở thành các giá trị vô cực. Cách này cho kết quả đúng khi gốc tia không nằm đúng trên mặt slab. Nếu gốc tia nằm chính xác trên mặt slab và $d_a = 0$, phép nhân $0 dot infinity$ có thể tạo ra `NaN`; khi đó cần xử lý riêng nếu muốn đảm bảo ổn định số học tuyệt đối.

Phép kiểm tra AABB là cơ sở để cắt tỉa cây BVH. Trong `BVHNode::traverse`, mỗi nút được kiểm tra bằng `node.aabb.hit(ray, EPSILON, t_max)`. Nếu kết quả là `false`, toàn bộ nhánh con của nút bị bỏ qua; nếu là `true`, thuật toán tiếp tục đi xuống nút lá để kiểm tra chi tiết với các hình học bên trong.

Độ phức tạp của một phép kiểm tra AABB là $O(1)$ vì chỉ cần xử lý ba trục.

== Thuật toán Moller–Trumbore (tia – tam giác)

=== Đầu vào
- Tia `ray` với gốc `origin` và hướng `direction`.
- Tam giác với ba đỉnh $arrow(v)_0, arrow(v)_1, arrow(v)_2$ và ba tọa độ kết cấu tương ứng $arrow("uv")_0, arrow("uv")_1, arrow("uv")_2$.
- Khoảng tham số hợp lệ $[t_min, t_max]$.

=== Đầu ra
- `true` nếu tia cắt tam giác trong khoảng tham số cho phép.
- Khi cắt, biến `tuv = (t, u, v)` chứa khoảng cách $t$ và hai tọa độ barycentric $u$, $v$ của giao điểm.

=== Phương pháp
Đặt hai cạnh của tam giác:

$ arrow(e)_1 = arrow(v)_1 - arrow(v)_0, quad arrow(e)_2 = arrow(v)_2 - arrow(v)_0 $

Mọi điểm trên tam giác có thể biểu diễn theo tọa độ barycentric:

$ arrow(p) = arrow(v)_0 + u arrow(e)_1 + v arrow(e)_2 $

với điều kiện $u >= 0$, $v >= 0$ và $u + v <= 1$. Giao điểm của tia và tam giác thỏa mãn:

$ arrow(o) + t arrow(d) = arrow(v)_0 + u arrow(e)_1 + v arrow(e)_2 $

Đây là hệ ba phương trình tuyến tính với ba ẩn $t$, $u$, $v$. Thuật toán Moller–Trumbore giải hệ này bằng quy tắc Cramer mà không cần tính trực tiếp phương trình mặt phẳng. Các đại lượng trung gian được định nghĩa như sau:

$ arrow(p)_"vec" = arrow(d) times arrow(e)_2 $

$ "det" = arrow(e)_1 dot arrow(p)_"vec" $

Nếu $|"det"| < epsilon$, tia gần như song song với mặt phẳng tam giác và không có giao điểm ổn định. Ngược lại, đặt:

$ arrow(t)_"vec" = arrow(o) - arrow(v)_0 $

$ u = (arrow(t)_"vec" dot arrow(p)_"vec") / "det" $

Nếu $u < 0$ hoặc $u > 1$, giao điểm nằm ngoài tam giác và phép kiểm tra dừng lại. Tiếp tục tính:

$ arrow(q)_"vec" = arrow(t)_"vec" times arrow(e)_1 $

$ v = (arrow(d) dot arrow(q)_"vec") / "det" $

Nếu $v < 0$ hoặc $u + v > 1$, giao điểm tiếp tục nằm ngoài tam giác. Khoảng cách theo tia được tính bởi:

$ t = (arrow(e)_2 dot arrow(q)_"vec") / "det" $

Cuối cùng, giao điểm chỉ được chấp nhận nếu $t_min <= t <= t_max$.

Thuật toán không loại bỏ mặt sau: `det` có thể âm, và điều này là cần thiết để phát hiện giao cắt từ cả hai phía của tam giác. Sau khi `RawTriangle::hit` trả về $t$, $u$, $v$, lớp `Triangle` tính pháp tuyến ngoài:

$ arrow(n)_"out" = "normalize"(arrow(e)_1 times arrow(e)_2) $

rồi xác định `front_face` giống như trường hợp mặt cầu, nhưng dùng hướng tia trong không gian cục bộ:

$ "front_face" = arrow(d)' dot arrow(n)_"out" < 0 $

Nếu `front_face` sai, pháp tuyến được đảo dấu. Tọa độ kết cấu tại giao điểm được nội suy tuyến tính với trọng số $w = 1 - u - v$:

$ arrow("uv") = w arrow("uv")_0 + u arrow("uv")_1 + v arrow("uv")_2 $

Sau đó điểm giao và pháp tuyến được biến đổi về không gian thế giới, còn khoảng cách được tính lại bằng hình chiếu lên tia thế giới giống như ở thuật toán tia – mặt cầu.

Trong lớp `Mesh`, thuật toán này được gọi cho từng tam giác nằm trong nút lá của BVH. Sau mỗi va chạm, `t_max` được cập nhật bằng khoảng cách gần nhất vừa tìm được, nhờ đó các tam giác ở xa hơn có thể bị loại bỏ sớm.

Độ phức tạp của một phép kiểm tra tia – tam giác là $O(1)$. Khi kết hợp với BVH, số tam giác cần kiểm tra trung bình giảm từ $O(N)$ xuống còn $O(log N)$.

== Thuật toán xây dựng BVH
=== Đầu vào
- Danh sách các vật thể trong không gian.
- Giới hạn số lượng vật thể tối đa cho phép trong một nút lá `n`.

=== Đầu ra
- Nút gốc của cây BVH.

=== Phương pháp
+ Tính toán AABB bao trọn toàn bộ danh sách các vật thể hiện tại.
+ Nếu số lượng vật thể trong danh sách nhỏ hơn hoặc bằng `n`:
  - Khởi tạo `BVHNode` hiện tại thành nút lá.
  - Gán danh sách vật thể cho nút lá này quản lý.
  - Kết thúc đệ quy và trả về nút hiện tại.
+ Nếu số lượng vật thể lớn hơn `n`, dựa vào kích thước (chiều dài, rộng, cao) của AABB tổng vừa tìm được, chọn ra trục không gian dài nhất (trục $X$, $Y$ hoặc $Z$).
+ Sắp xếp danh sách các vật thể theo thứ tự tăng dần dựa trên tọa độ trọng tâm của chúng dọc theo trục vừa chọn. Sau đó, chia danh sách đã sắp xếp thành hai nửa bằng nhau (nửa trái và nửa phải).
+ Gọi đệ quy thuật toán này để xây dựng hai nhánh con:
  - Khởi tạo nút `left` từ nửa danh sách bên trái.
  - Khởi tạo nút `right` từ nửa danh sách bên phải.
+ Tạo `BVHNode` hiện tại với hai con là `left` và `right`. Cập nhật hộp bao AABB của nút này bằng AABB của nút `left` và nút `right`. Trả về nút hiện tại.

== Thuật toán truy vấn giao cắt trên cây BVH
=== Đầu vào
- Nút BVH hiện tại cần kiểm tra (bắt đầu bằng nút gốc).
- Tia sáng `ray` (bao gồm vị trí gốc và vector hướng).
- Khoảng cách va chạm gần nhất tính đến thời điểm hiện tại `t_max` (dùng để tối ưu hóa, bỏ qua các vật thể nằm xa hơn điểm đã va chạm trước đó).

=== Đầu ra
- Trạng thái giao cắt (`true` nếu có cắt, `false` nếu không).
- Cấu trúc `HitInfo` chứa thông tin chi tiết về điểm giao cắt gần nhất (nếu có).

=== Phương pháp
+ Kiểm tra giao cắt giữa tia `ray` và hộp bao `AABB` của nút hiện tại. Nếu tia không cắt hộp bao, hoặc điểm giao cắt lớn hơn `t_max`, lập tức dừng đệ quy và trả về `false`.
+ Nếu nút hiện tại là nút lá:
  - Khởi tạo cờ `hit_anything = false`.
  - Duyệt tuần tự qua mảng các vật thể nằm bên trong nút lá.
  - Gọi thuật toán kiểm tra giao cắt chi tiết của từng vật thể với tia `ray`.
  - Nếu có va chạm gần hơn `t_max`, ghi nhận thông tin vào `HitInfo`, cập nhật lại `t_max` bằng khoảng cách va chạm mới, và gán `hit_anything = true`.
  - Trả về cờ `hit_anything`.
+ Nếu nút hiện tại là nút nhánh (có hai nút con `left` và `right`):
  - Gọi đệ quy thuật toán này trên nhánh `left`. Nếu có va chạm, `t_max` sẽ tự động được thu hẹp lại.
  - Gọi đệ quy thuật toán này trên nhánh `right` với giới hạn `t_max` mới nhất (giúp nhánh `right` có thể thoát sớm nếu mọi vật thể bên phải đều xa hơn điểm đã cắt bên trái).
  - Trả về `true` nếu có xảy ra va chạm ở bất kỳ nhánh con nào.
