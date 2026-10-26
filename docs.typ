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
- Xây dựng thành công một trình dò tia trên CPU có khả năng mô phỏng các định luật quang học và kết xuất hình ảnh chân thực, với hiệu xuất chấp nhận được.

= Nhiệm vụ
== Về tính toán
- Cài đặt hệ thống hình học toán học cơ bản (Vector, Tia, Hộp bao AABB, Hình cầu, Tam giác).
- Xử lý tính toán giao cắt tia với các hình học cơ bản (mặt cầu) và phức tạp (lưới đa giác tạo từ nhiều tam giác).
- Mô phỏng các tính chất vật liệu quang học: phản xạ gương (kim loại), khúc xạ (thủy tinh/nước), tán xạ khuếch tán (bề mặt nhám), suy hao và tán xạ trong môi trường thuần nhất (nhựa, khói).
- Tính toán ánh sáng toàn cục bằng thuật toán Path Tracing dựa trên mô phỏng Monte Carlo.
- Ứng dụng cơ chế lấy mẫu quan trọng trong lý thuyết xác suất thống kê để hội tụ hình ảnh kết xuất nhanh hơn.

== Về cấu trúc dữ liệu
- Cài đặt cấu trúc dữ liệu cây phân vùng không gian để tăng tốc độ truy vấn giao điểm giữa tia và hàng ngàn vật thể trong không gian.
- Ứng dụng đa luồng để phân chia công việc tính toán các điểm ảnh lên nhiều lõi CPU.

== Về giao diện
- Xây dựng không gian làm việc trực quan bằng SFML và ImGui.
- Xây dựng các chức năng tương tác thời gian thực:
  - Thay đổi thuộc tính vật thể (vị trí, góc quay, tỉ lệ, bán kính, màu sắc, chiết suất, độ nhám, cường độ sáng).
  - Điều hướng Camera (vị trí, điểm nhìn, tiêu cự, khẩu độ, phơi sáng).
  - Chọn vật thể trực tiếp trên khung hình.
  - Thay đổi độ phân giải khung hình, số lần dội tối đa và số mẫu mỗi điểm ảnh.
  - Tạm dừng/tiếp tục và đặt lại quá trình tích lũy.

= Thư viện sử dụng
- Thư viện đồ họa SFML kết hợp với thư viện giao diện người dùng ImGui.
- Thư viện đa luồng OpenMP được cài đặt sẵn trong gcc/clang/msvc.
- Thư viện chuẩn C++: `std::vector` cho mảng động, `std::random` để sinh số ngẫu nhiên, `std::thread`/`std::atomic`/`std::mutex` để quản lý luồng và đồng bộ hóa.

= Cấu trúc dữ liệu
== Tổng quan các lớp

Các sơ đồ lớp dưới đây được xây dựng trực tiếp từ cấu trúc của các tệp tiêu đề (`.h`) trong thư mục `src/`. Hệ thống được chia thành bốn nhóm chính: kiến trúc ứng dụng và cảnh, hệ thống hình học, hệ thống vật liệu – kết cấu – môi trường, và các kiểu toán học hỗ trợ.

#text(size: 9pt)[
  *Ký hiệu:* ◆ đối tượng sở hữu (composition); ◇ đối tượng tham chiếu (aggregation); ▷ kế thừa (inheritance); ⇢ phụ thuộc (dependency). Một số chữ ký phương thức dài, các thuộc tính/phương thức hỗ trợ đã được lược bỏ và viết gọn thành `...` để đơn giản hóa biểu đồ; chi tiết đầy đủ có trong các tệp tiêu đề.
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
            "- camera: Camera&",
            "- scene: Scene&",
            "- tracer: RayTracer",
            "- window: sf::RenderWindow",
            "- render_thread: thread",
            "- scene_mutex: mutex",
            "- is_running, is_paused, is_data_ready: atomic<bool>",
            "- lock_requested: atomic<bool>",
            "- pending_actions: vector<function<void()>>",
            "- front_pixels: vector<uint32_t>",
            "...",
          ),
          (
            "+ run()",
            "- renderRoutine()",
            "- postAction(action)",
            "- applyPendingActions()",
            "...",
          )
        ), name: <gui>),

        node((1, 0), class(
          "RayTracer",
          (
            "- viewport_width, viewport_height: unsigned",
            "- max_bounces: unsigned = 10",
            "- samples_per_pixel: unsigned = 4",
            "- pass: unsigned",
            "- buffer: vector<Color>",
            "- pixels: vector<uint32_t>",
          ),
          (
            "+ RayTracer(width, height)",
            "+ getData(): const uint8_t*",
            "+ trace(ray: Ray, scene: const Scene&): Color",
            "+ reset()",
            "+ render(camera: const Camera&, scene: const Scene&)",
          )
        ), name: <raytracer>),

        node((-1, 1.3), class(
          "Camera",
          (
            "- focal_length, exposure, aperture: float",
            "- position, lookat: Vec3",
            "- w, u, v: Vec3",
          ),
          (
            "+ Camera(focal_length, position, lookat)",
            "+ getRayAt(uv: Vec2, jitter: bool = true): Ray",
            "+ lookAt(v: const Vec3&)",
            "- computeOrientation()",
          )
        ), name: <camera>),

        node((1, 1.3), class(
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
            "+ getClosest(ray, obj): HitInfo",
            "+ buildBVH()",
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
            "+ {static} flatten(flat_bvh, node, offset: int&): int",
            "+ {static} traverse(flat_bvh, ray, hitFunc): HitInfo",
          )
        ), name: <bvhnode>),

        node((1, 3), class(
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
  ], caption: [Kiến trúc ứng dụng và cảnh.]
)

`RayTracer` là thư viện độc lập với giao diện: nó chỉ nhận `Camera` và `Scene` khi chạy phương thức `render()`. Điều này giúp cho người sử dụng thư viện có thể dựng/chuyển cảnh, kết xuất ảnh linh hoạt thông qua mã lệnh mà không cần giao diện người dùng.

`Gui` sở hữu một `RayTracer`, nó chạy vòng lặp kết xuất và vòng lặp giao diện trên hai luồng riêng biệt. Vì cả 2 luồng đều có thể truy cập đến dữ liệu cảnh như `scene`, `tracer` và `camera`, `Gui` còn sở hữu `scene_mutex` để khóa cảnh, ngăn việc 2 luồng này cùng truy cập và thay đổi dữ liệu cùng một lúc. Luồng kết xuất chỉ hoạt động khi nó giữ `scene_mutex`, trong khi luồng giao diện luôn hoạt động trong suốt vòng đời chương trình. Mọi thay đổi từ giao diện được đưa vào `pending_actions` và chỉ áp dụng khi luồng giao diện lấy được `scene_mutex`; sau khi áp dụng hết các sửa đổi trong `pending_actions` thì nó ngay lập tức thả `scene_mutex` để luồng kết xuất hoạt động trở lại. Luồng giao diện có thể yêu cầu luồng kết xuất thả `scene_mutex` bằng cờ `lock_requested`. Sau mỗi lượt, luồng kết xuất sao chép ảnh từ `tracer` sang `front_pixels` rồi bật cờ `is_data_ready`; luồng giao diện chỉ đọc bộ đệm này khi cờ đã bật, sau đó mới tải lên `sf::Texture`. Thiết kế này giúp luồng giao diện luôn luôn hoạt động mà không bị dừng lại, làm cho trải nghiệm người dùng mượt mà.

#figure(
  block(breakable: false, width: 100%)[
    #align(center)[
      #diagram(
        node-shape: rect,
        spacing: (0.2cm, 0.2cm),

        node((0, 0), class(
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

        node((-1, 0), class(
          "Hittable «abstract»",
          (
            "# transform: Transform",
            "# material: Material*",
            "# name: string",
          ),
          (
            "+ hit(ray: const Ray&): HitInfo = 0",
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
            "- radius: float",
          ),
          (
            "+ Sphere(name, material, radius)",
            "+ hit(ray: const Ray&): HitInfo",
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
            "+ hit(ray: const Ray&): HitInfo",
            "# getLocalAABB(): AABB",
            "- computeAABB()",
          )
        ), name: <mesh>),

        node((-1, 3.5), class(
          "RawTriangle",
          (
            "+ v0, v1, v2: Vec3",
            "+ uv0, uv1, uv2: Vec2",
          ),
          (
            "+ RawTriangle(v0, v1, v2)",
            "+ RawTriangle(v0, v1, v2, uv0, uv1, uv2)",
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

        edge(<hittable>, <transform>, marks: composition),
        edge(<sphere>, <hittable>, marks: inheritance),
        edge(<mesh>, <hittable>, marks: inheritance),
        edge(<mesh>, <rawtriangle>, marks: composition),
        edge(<mesh>, <linearbvh>, marks: composition, bend: 12deg),
      )
    ]
  ], caption: [Hệ thống hình học.]
)

Hệ thống hình học được xây dựng quanh lớp cơ sở trừu tượng `Hittable`. Mỗi vật thể lưu một `Transform` (gồm tỉ lệ, ma trận quay `Mat3x3`, góc quay và vị trí) cùng con trỏ tới `Material`. Lớp này khai báo hai phương thức thuần ảo `hit()` và `getLocalAABB()`; các lớp dẫn xuất chỉ cần cài đặt phép giao cắt trong không gian cục bộ, còn `getAABB()` sẽ áp dụng `Transform` để trả về hộp bao trong không gian thế giới.

- `Sphere` chỉ lưu bán kính; tâm của nó nằm tại gốc không gian cục bộ, còn vị trí trong thế giới được xác định bởi `Transform.translation`.
- `Mesh` quản lý một danh sách `RawTriangle` và một BVH phẳng (`LinearBVHNode`) để tăng tốc truy vấn giao cắt trên lưới lớn, đồng thời cài đặt thuật toán Moller–Trumbore để kiểm tra giao cắt tia–tam giác và nội suy tọa độ `uv`.
- `RawTriangle` chỉ lưu dữ liệu hình học (các đỉnh và tọa độ kết cấu), không chứa logic giao cắt.

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
            "+ Material(name, texture, medium)",
            "+ scatter(...): ScatterResult",
            "+ emitted(rec): Color",
            "+ getName(): string&",
            "+ getTexture(): Texture*",
            "+ getMedium(): Medium*",
          )
        ), name: <material>),

        node((-1, 3.9), class(
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
  ], caption: [Hệ thống vật liệu.]
)

`Material` định nghĩa giao diện chung cho các loại vật liệu. `scatter` nhận tia hiện tại, màu suy hao bề mặt, `HitInfo` và môi trường hiện tại, rồi trả về một trong ba trạng thái `ScatterResult`: `Terminated` (dừng tia), `Scattered` (tia tiếp tục trong cùng môi trường) hoặc `Refracted` (tia vừa đi qua biên giới môi trường). Cài đặt mặc định của `Material::scatter` trả về `Terminated`, còn `Material::emitted` trả về màu đen. `Matte` tán xạ khuếch tán quanh pháp tuyến, `Metal` phản xạ gương có độ nhám, `Dielectric` khúc xạ theo định luật Snell, và `Light` chỉ phát xạ (không tán xạ) nên kết thúc tia tại bề mặt.

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
            "+ get(rec): Color = 0",
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
            "+ Material(name, texture, medium)",
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
            "+ Medium(name, ior)",
            "+ {virtual} transmit(distance): Color",
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
            "+ get(rec): Color",
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
            "+ transmit(distance): Color",
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
  ], caption: [Kết cấu, vật liệu và môi trường truyền dẫn.]
)

`Texture` cung cấp dữ liệu bề mặt tại một `HitInfo`; `ColorTexture` là cài đặt đơn giản nhất, luôn trả về một màu không đổi. `Medium` mô tả môi trường mà tia đang truyền qua. `Medium::transmit(distance)` trả về hệ số suy hao năng lượng trên một đoạn đường dài `distance`; lớp cơ sở trả về màu trắng (không hấp thụ), còn `Homogeneous` cài đặt định luật Beer–Lambert:

$ "transmit"(d) = exp(-sigma dot "density" dot d) $

với phép nhân và hàm mũ được áp dụng cho từng kênh màu. Môi trường toàn cục của cảnh được lưu trong `Scene.global_medium`; mỗi vật liệu `Dielectric` có thể tham chiếu thêm một `Medium` riêng để mô tả môi trường bên trong vật thể.

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
            "+ Vec2(v: const Vec3&)",
            "+ operator[](axis): float&",
            "+ operator+(v), operator-(v)",
            "+ operator*(t), operator/(t): Vec2",
            "+ operator*(v), operator/(v): Vec2",
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
            "+ Ray(origin = Vec3(0), direction = Vec3(0, 0, -1))",
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
            "+ AABB(p: Vec3 = Vec3(0))",
            "+ AABB(points: initializer_list<Vec3>)",
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
            "+ {static} pointInCircle(radius = 1.0f): Vec2",
            "+ {static} vec3(): Vec3",
            "+ {static} vec2(): Vec2",
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
  ], caption: [Các kiểu toán học hỗ trợ.]
)

Các kiểu toán học được thiết kế dưới dạng `struct` nhỏ, thao tác trên giá trị và không sở hữu tài nguyên. `Vec3` kiêm luôn vai trò `Color` (typedef) và cung cấp các phép toán vector, phản xạ, khúc xạ, chuyển đổi màu sang ABGR. `Ray` và `AABB` là hai kiểu nền tảng cho mọi phép truy vấn giao cắt; `RawTriangle` kết hợp `Vec3` và `Vec2` để lưu hình học và tọa độ kết cấu. `Mat3x3` và `Transform` đóng gói phép biến đổi affine, còn `RNG` là lớp tiện ích tĩnh phục vụ lấy mẫu ngẫu nhiên trong path tracing.

#pagebreak()

== Cây phân vùng không gian Bounding Volume Hierarchy (BVH)
Thay vì duyệt tuyến tính qua toàn bộ danh sách vật thể để tìm giao điểm, hệ thống sử dụng cấu trúc cây phân vùng BVH nhằm tối ưu hóa độ phức tạp thời gian truy vấn từ $O(N)$ xuống trung bình $O(log N)$.

=== Thuật toán xây dựng cây
Cây được dựng tạm bằng con trỏ theo phương pháp chia để trị, sau đó làm phẳng thành mảng `LinearBVHNode`:
+ Xác định hộp bao (AABB - Axis-Aligned Bounding Box) bao trọn toàn bộ các vật thể hiện có.
+ Tìm trục không gian dài nhất của hộp bao (X, Y hoặc Z) để làm trục phân chia.
+ Sắp xếp các vật thể theo tọa độ nhỏ nhất của AABB dọc theo trục này, rồi chia thành hai nhóm (nửa trái và nửa phải).
+ Tạo hai nút con (`BVHNode`) cho hai nhóm và tiếp tục đệ quy cho đến khi số vật thể trong nút không vượt quá `PRIM_PER_LEAF` (4). Nút đó trở thành nút lá.
+ Duyệt cây theo thứ tự trước để ghi vào mảng phẳng: nút lá lưu `primitives_offset`/`num_objects`, nút nhánh lưu `right_offset`/`axis` với `num_objects = 0`.

=== Thuật toán truy vấn giao cắt
Khi một tia được bắn ra, chương trình duyệt mảng phẳng bằng một ngăn xếp chỉ số nút:
- Nếu tia *không cắt* hộp bao AABB của nút: bỏ qua nút đó và lấy nút kế tiếp từ ngăn xếp.
- Nếu *có cắt* và nút là nút nhánh: đẩy nút con phải vào ngăn xếp rồi đi xuống nút con trái.
- Nếu *có cắt* và nút là nút lá: kiểm tra giao cắt chi tiết với từng hình học (Sphere, Mesh) trong nút, cập nhật khoảng cách gần nhất `t_max` sau mỗi va chạm.

= Thuật toán

== Thuật toán dò tia cơ bản.

=== Đầu vào
- Tia sơ cấp `ray`.
- Cấu trúc dữ liệu chứa các vật thể cần kết xuất (cây BVH phẳng), cùng môi trường toàn cục `scene.global_medium`.
- Số lần dội tối đa `max_bounces`.

=== Đầu ra
- Màu của tia sau khi dò.

=== Phương pháp
- *Khởi tạo:* đặt `final_color = (0, 0, 0)`, `throughput = (1, 1, 1)` và một ngăn xếp môi trường `medium_stack` chứa môi trường toàn cục `scene.getGlobalMedium()`.
- *Dò tia:* lặp tối đa `max_bounces` lần. Ở mỗi lần dội, thực hiện lần lượt các bước sau:
  - *Kiểm tra giao cắt:* tìm giao điểm gần nhất của `ray` với các vật thể. Nếu tia không cắt vật thể nào thì tính toán ánh sáng nền và thoát khỏi vòng lặp.

  - *Suy hao theo môi trường:* `throughput *= current_medium->transmit(distance)`, với `current_medium` là đỉnh của `medium_stack`. Với môi trường đồng chất `Homogeneous`, hàm `transmit(distance)` chính là định luật Beer–Lambert trên đoạn đường `distance`.

  - *Phát xạ:* `final_color += throughput * material->emitted(rec)`. `Material::emitted` mặc định trả về màu đen; `Light` trả về `texture->get(rec) * emission_strength`.

  - *Tính toán tia mới:* khởi tạo `surface_attenuation = (1, 1, 1)` rồi gọi `material->scatter(ray, surface_attenuation, rec, current_medium)`; hàm này vừa tính toán suy hao bề mặt, vừa tính toán hướng tia mới (dựa trên thuật toán tán xạ/khúc xạ) và trả về kết quả tán xạ/khúc xạ/kết thúc dò tia. Nếu như kết quả là
    + `Terminated`: tia gặp nguồn sáng, thoát khỏi vòng lặp.
    + `Scattered`: tia bị tán xạ, nhân `throughput *= surface_attenuation` rồi tiếp tục với tia mới
    + `Refracted`: cập nhật `medium_stack`: nếu cắt mặt ngoài của vật liệu thì đẩy môi trường của vật liệu vào; nếu không và đỉnh ngăn xếp đúng là môi trường đó thì lấy ra.
  - *Russian roulette:* khi chỉ số dội `bounces` đủ lớn, đặt `m = max(throughput.x, throughput.y, throughput.z)`. Nếu `RNG::uniform() > m` thì kết thúc tia; ngược lại chia `throughput` cho `m` để bảo toàn năng lượng kỳ vọng. Điều này giúp thiên vị các tia mang ánh sáng mạnh, kết thúc sớm các tia mang ánh sáng yếu nhằm giảm thời gian dò tia cho các tia đem lại hiệu quả thấp.
  - *Lặp lại:* với tia `ray` vừa cập nhật, quá trình tiếp tục cho đến khi tia kết thúc hoặc đạt giới hạn số lần dội.
- Sau khi thoát khỏi vòng lặp, trả về `final_color` là màu đã dò được từ tia sơ cấp ban đầu.

== Thuật toán khúc xạ
=== Đầu vào
- Hướng tia tới $arrow(d)$
- Pháp tuyến trên vật liệu tại tiếp điểm $arrow(n)$
- Mặt tiếp xúc vật thể (mặt trong hoặc ngoài)
- Chiết xuất của môi trường chứa tia $n_1$
- Chiết xuất của môi trường tia hướng đến $n_2$

=== Đầu ra
- Hướng tia mới $arrow(d_"new")$
- Kết quả: bị tán xạ `Scattered` hoặc bị khúc xạ `Refracted`.

=== Phương pháp
Tỉ số chiết suất được lấy từ môi trường hiện tại và môi trường bên trong vật liệu:
$ eta = n_1 / n_2 $
Ta có:
$ cos theta = min(-arrow(d) dot arrow(n), 1) $
Xác suất phản xạ được xấp xỉ bằng công thức Schlick:
$ R = R_0 + (1 - R_0)(1 - cos theta)^5, quad R_0 = ((1 - eta)/(1 + eta))^2 $
Nếu xảy ra phản xạ toàn phần $eta^2 (1 - cos^2 theta) > 1$ hoặc $R > X$ ($X$ là biến ngẫu nhiên phân phối đều từ $0$ đến $1$) và $eta != 1$, tia phản xạ và không thay đổi môi trường:
$ arrow(d)_"new" = arrow(d) - 2(arrow(d) dot arrow(n))arrow(n) $
Khi này kết quả là `Scattered`. Ngược lại, tia khúc xạ qua bề mặt và kết quả là `Refracted`:
$ arrow(d)_"new" = eta arrow(d) + (eta cos theta - sqrt(1 - eta^2 (1 - cos^2 theta))) arrow(n) $

== Thuật toán tán xạ
=== Đầu vào
- Hướng tia ban đầu $arrow(d)$
- Độ nhám vật liệu $r$ có giá trị từ $0$ đến $1$

=== Đầu ra
- Hướng tia mới $arrow(d_"new")$
- Kết quả: luôn bị tán xạ `Scattered`.

=== Phương pháp
Với vật liệu kim loại `Metal`, hướng phản xạ gương được tính bởi:
$ arrow(d)_"spec" = arrow(d) - 2(arrow(d) dot arrow(n))arrow(n) $
Khi độ nhám $r > 0$, một vector ngẫu nhiên $arrow(v)_"rand"$ (chưa chuẩn hóa) được nhân với $r$ rồi cộng vào hướng phản xạ:
$ arrow(d)_"new" = "normalize"(arrow(d)_"spec" + r arrow(v)_"rand") $
Nếu $arrow(d)_"new" dot arrow(n) <= 0$, tức hướng mới quay ra sau bề mặt, chương trình phản xạ lại vector này qua $arrow(n)$ trước khi chuẩn hóa.

Vật liệu nhám `Matte` chính là trường hợp đặc biệt của vật liệu kim loại với độ nhám $r = 1$. Do đó thuật toán sinh tia tán xạ của `Matte` về cơ bản là thuật toán sinh tia tán xạ cua `Metal` nhưng được tối ưu riêng cho trường hợp này:
$ arrow(d)_"new" = "normalize"(arrow(n) + arrow(v)_"rand") $
trong đó $arrow(v)_"rand"$ là một vector đơn vị ngẫu nhiên.

== Thuật toán giao cắt tia – mặt cầu

=== Đầu vào
- Tia `ray` trong không gian thế giới, gồm gốc `origin` và hướng `direction`.
- Mặt cầu `Sphere` với bán kính `radius` và phép biến đổi `transform`. Trong không gian cục bộ, mặt cầu luôn có tâm tại gốc tọa độ; phép dịch được lưu trong `transform.translation`.

=== Đầu ra
- Một cấu trúc `HitInfo` chứa khoảng cách `distance`, điểm giao `hit_point`, pháp tuyến `normal`, cờ `front_face` và vật liệu của mặt cầu.
- Nếu tia không cắt mặt cầu, `distance` giữ giá trị mặc định `-1` và macro `DID_HIT` trả về `false`.

=== Phương pháp
Để hỗ trợ dịch, quay và co giãn vật thể, phép kiểm tra được thực hiện trong không gian cục bộ của mặt cầu. Gốc và hướng của tia lần lượt được đưa về không gian này bằng nghịch đảo của `Transform`:

$ arrow(o)' = T^(-1)(arrow(o)), quad arrow(d)' = T^(-1)(arrow(d)) $

Vì tâm mặt cầu nằm tại gốc không gian cục bộ, đặt $arrow(m) = arrow(o)'$. Điều kiện để một điểm trên tia nằm trên mặt cầu là:

$ ||arrow(m) + t arrow(d)'||^2 = r^2 $

Khai triển và thu gọn phương trình bậc hai theo $t$:

$ (arrow(d)' dot arrow(d)')t^2 + 2(arrow(m) dot arrow(d)')t + (arrow(m) dot arrow(m) - r^2) = 0 $

Chương trình sử dụng dạng rút gọn với nửa hệ số bậc nhất để giảm số phép nhân:

$ a = arrow(d)' dot arrow(d)', quad b_"half" = -(arrow(m) dot arrow(d)'), quad c = arrow(m) dot arrow(m) - r^2 $

$ Delta = b_"half"^2 - a c $

Nếu $Delta < 0$, tia không cắt mặt cầu. Ngược lại, hai nghiệm là:

$ t_0 = (b_"half" - sqrt(Delta))/a, quad t_1 = (b_"half" + sqrt(Delta))/a $

Chương trình ưu tiên nghiệm nhỏ hơn, $t_0$. Nếu $t_0 < epsilon$, nghĩa là giao điểm nằm phía sau tia hoặc quá gần gốc tia, nghiệm $t_1$ được chọn thay thế. Nếu cả hai nghiệm đều nhỏ hơn $epsilon$, phép kiểm tra thất bại. Bước này vừa tránh hiện tượng tự cắt (shadow acne), vừa xử lý trường hợp gốc tia nằm bên trong mặt cầu.

Sau khi có khoảng cách $t$, điểm giao và pháp tuyến ngoài trong không gian cục bộ được tính bởi:

$ arrow(p)' = arrow(o)' + t arrow(d)', quad arrow(n)_"out" = arrow(p)'/r $

Cờ `front_face` cho biết tia đang đi vào mặt trước hay mặt sau của mặt cầu:

$ "front_face" = arrow(d)' dot arrow(n)_"out" < 0 $

Nếu `front_face` đúng, pháp tuyến được giữ nguyên; ngược lại, pháp tuyến được đảo dấu để luôn hướng về phía tia tới. Cuối cùng, điểm giao và pháp tuyến được đưa trở lại không gian thế giới:

$ arrow(p) = T(arrow(p)'), quad arrow(n) = N(arrow(n)') $

Khoảng cách cuối cùng được tính lại bằng hình chiếu của vector từ gốc tia đến điểm giao lên hướng tia thế giới:

$ t = (arrow(p) - arrow(o)) dot arrow(d) $

Cách tính lại này cần thiết vì `Transform` có thể chứa phép co giãn, khiến khoảng cách trong không gian cục bộ không còn trùng với khoảng cách thế giới.

Hộp bao cục bộ của mặt cầu được xây dựng đơn giản từ bán kính, vì tâm cục bộ là gốc tọa độ:

$ "AABB"_"local" = [-(r, r, r), (r, r, r)] $

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

== Thuật toán giao cắt tia - tam giác (Moller–Trumbore)

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

Thuật toán không loại bỏ mặt sau: `det` có thể âm, và điều này là cần thiết để phát hiện giao cắt từ cả hai phía của tam giác. Sau khi tính được $t$, $u$, $v$, `Mesh` tính pháp tuyến ngoài:

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
- Danh sách các vật thể (hoặc tam giác) trong không gian.
- Ngưỡng số vật thể tối đa trong một nút lá, `PRIM_PER_LEAF = 4`.

=== Đầu ra
- Mảng phẳng `flat_bvh: vector<LinearBVHNode>` dùng cho truy vấn, được lưu trong `Scene` hoặc `Mesh`.

=== Phương pháp
Quá trình xây dựng gồm hai bước: dựng cây tạm bằng con trỏ, sau đó làm phẳng thành mảng.

+ *Dựng cây tạm:*
  + Tính AABB bao trọn toàn bộ danh sách vật thể hiện tại.
  + Nếu số lượng vật thể nhỏ hơn hoặc bằng `PRIM_PER_LEAF`, đánh dấu nút hiện tại là nút lá, lưu `start_index` và `num_objects`, rồi kết thúc đệ quy.
  + Ngược lại, chọn trục có chiều dài lớn nhất của AABB tổng (trục $X$, $Y$ hoặc $Z$).
  + Sắp xếp các vật thể tăng dần theo tọa độ nhỏ nhất (`minp[axis]`) của AABB tương ứng trên trục vừa chọn. Chương trình dùng `minp` chứ không dùng trọng tâm.
  + Chia danh sách đã sắp xếp thành hai nửa bằng nhau và gọi đệ quy để tạo `left`, `right`. AABB của nút hiện tại là hợp AABB của hai nút con.

+ *Làm phẳng:* `BVHNode::flatten` duyệt cây theo thứ tự trước (pre-order) và ghi mỗi nút vào một `LinearBVHNode`:
  + Nút lá lưu `primitives_offset` (chỉ số vật thể đầu tiên) và `num_objects > 0`.
  + Nút nhánh lưu `num_objects = 0`, trục phân chia `axis` và `right_offset` (chỉ số nút con phải); nút con trái luôn nằm ngay sau nút hiện tại.
  + Mảng được cấp phát tạm `2N` phần tử rồi thu gọn về đúng số nút đã ghi. Sau đó cây tạm bằng con trỏ được giải phóng.

== Thuật toán truy vấn giao cắt trên cây BVH
=== Đầu vào
- Mảng phẳng `flat_bvh` (bắt đầu từ nút 0).
- Tia sáng `ray`.
- Hàm callback `hitFunc(primitive_idx, closest_hit)` để kiểm tra chi tiết từng vật thể trong nút lá.

=== Đầu ra
- `HitInfo` chứa thông tin về điểm giao gần nhất (nếu có); `distance = -1` nếu không cắt.

=== Phương pháp
Chương trình duyệt lặp bằng một ngăn xếp chỉ số nút `nodes_to_visit[64]`, không đệ quy:

+ Khởi tạo `current_node_index = 0`, `to_visit_offset = 0`, `closest_hit` rỗng.
+ Lặp:
  + Tính `t_max` bằng khoảng cách va chạm gần nhất nếu đã có, ngược lại bằng `FAR_DISTANCE`.
  + Kiểm tra `node.aabb.hit(ray, EPSILON, t_max)`:
    - Nếu không cắt: lấy nút tiếp theo từ ngăn xếp; nếu ngăn xếp rỗng thì kết thúc.
    - Nếu cắt và là nút lá (`num_objects > 0`): gọi `hitFunc` cho từng vật thể trong nút; `closest_hit` và `t_max` được cập nhật sau mỗi va chạm. Sau đó lấy nút tiếp theo từ ngăn xếp hoặc kết thúc.
    - Nếu cắt và là nút nhánh: đẩy `right_offset` vào ngăn xếp rồi đi xuống nút con trái (`current_node_index + 1`).
+ Trả về `closest_hit`.

Nhờ `t_max` được thu hẹp sau mỗi va chạm, các nút có AABB nằm xa hơn điểm giao gần nhất bị loại bỏ sớm. Độ phức tạp trung bình là $O(log N)$ cho mỗi tia.
