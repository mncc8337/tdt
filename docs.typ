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
#let dependency = "-->"

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
- Mô phỏng các tính chất vật liệu quang học: phản xạ gương (kim loại), khúc xạ (thủy tinh/nước), tán xạ khuếch tán, suy hao và tán xạ trong môi trường thuần nhất (nhựa, khói).
- Tính toán ánh sáng toàn cục bằng thuật toán Path Tracing dựa trên mô phỏng Monte Carlo.
- Ứng dụng cơ chế lấy mẫu quan trọng trong lý thuyết xác suất thống kê để hội tụ hình ảnh kết xuất nhanh hơn.

== Về cấu trúc dữ liệu
- Thiết kế mô hình các đối tượng linh hoạt, có tính đóng gói cao và dễ dàng mở rộng/tái sử dụng.
- Cài đặt cấu trúc dữ liệu cây phân vùng không gian để tăng tốc độ truy vấn giao điểm giữa tia và hàng ngàn vật thể trong không gian.
- Ứng dụng đa luồng để phân chia công việc tính toán các điểm ảnh lên nhiều lõi CPU.

== Về giao diện
- Xây dựng không gian làm việc trực quan bằng SFML và ImGui.
- Xây dựng các chức năng tương tác thời gian thực:
  - Lưu và tải cảnh.
  - Thay đổi thuộc tính vật thể (vị trí, góc quay, tỉ lệ, bán kính, màu sắc, chiết suất, độ nhám, cường độ sáng).
  - Điều hướng Camera (vị trí, điểm nhìn, tiêu cự, khẩu độ, phơi sáng).
  - Chọn vật thể trực tiếp trên khung hình.
  - Thay đổi độ phân giải khung hình, số lần dội tối đa và số mẫu mỗi điểm ảnh.
  - Tạm dừng/tiếp tục và đặt lại quá trình tích lũy.

= Thư viện sử dụng
- Thư viện đồ họa SFML kết hợp với thư viện giao diện người dùng ImGui.
- Thư viện đa luồng OpenMP được cài đặt sẵn trong GCC/Clang/MSVC.
- Thư viện chuẩn C++: `std::vector` cho mảng động, `std::random` để sinh số ngẫu nhiên, `std::thread`/`std::atomic`/`std::mutex` để quản lý luồng và đồng bộ hóa.

= Cấu trúc dữ liệu

== Tổng quan các lớp
Các sơ đồ lớp dưới đây được xây dựng trực tiếp từ cấu trúc của các tệp tiêu đề trong thư mục `src/`. Hệ thống được chia thành bốn nhóm chính: kiến trúc ứng dụng và cảnh, hệ thống hình học, hệ thống vật liệu – kết cấu – môi trường, và các kiểu toán học hỗ trợ.

#text(size: 9pt)[
  *Ký hiệu:* ◆ đối tượng sở hữu (composition); ◇ đối tượng tham chiếu (aggregation); ▷ kế thừa (inheritance); ⇢ phụ thuộc (dependency). `#` thuộc tính; `+` phương thức công khai; `-` phương thức không công khai. Chiều mũi tên: whole → part (composition/aggregation); lớp dẫn xuất → lớp cơ sở (inheritance); client → supplier (dependency). Một số chữ ký phương thức dài, các thuộc tính/phương thức hỗ trợ đã được lược bỏ và viết gọn thành `...` để đơn giản hóa biểu đồ; chi tiết đầy đủ có trong các tệp tiêu đề.
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
            "# camera: Camera&",
            "# scene: Scene&",
            "# tracer: RayTracer",
            "# window: sf::RenderWindow",
            "# render_thread: thread",
            "# scene_mutex: mutex",
            "# is_running, is_paused, is_data_ready: atomic<bool>",
            "# lock_requested: atomic<bool>",
            "# pending_actions: vector<function<void()>>",
            "# front_pixels: vector<uint32_t>",
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
            "# viewport_width, viewport_height: unsigned",
            "# max_bounces: unsigned = 10",
            "# samples_per_pixel: unsigned = 4",
            "# pass: unsigned",
            "# buffer: vector<Color>",
            "# pixels: vector<uint32_t>",
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
            "# focal_length, exposure, aperture: float",
            "# position, lookat: Vec3",
            "# w, u, v: Vec3",
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
            "# objects: vector<unique_ptr<Hittable>>",
            "# materials: vector<unique_ptr<Material>>",
            "# textures: vector<unique_ptr<Texture>>",
            "# mediums: vector<unique_ptr<Medium>>",
            "# global_medium: Medium*",
            "# bvh_root: unique_ptr<BVHNode>",
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
            "# aabb: AABB",
            "# left, right: unique_ptr<BVHNode>",
            "# start_index, num_objects: int",
          ),
          (
            "+ BVHNode(objects, start, end)",
            "+ isLeaf(): bool",
            "+ traverse(ray, hitFunc): HitInfo",
          )
        ), name: <bvhnode>),

        edge(<gui>, <raytracer>, marks: composition),
        edge(<gui>, <camera>, marks: aggregation),
        edge(<gui>, <scene>, marks: aggregation, bend: -12deg),
        edge(<raytracer>, <camera>, marks: dependency, bend: 10deg),
        edge(<raytracer>, <scene>, marks: dependency),

        edge(<scene>, <bvhnode>, marks: composition, bend: -10deg),
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
            "# scale: Vec3",
            "# rotation: Mat3x3",
            "# rotation_angles: Vec3",
            "# translation: Vec3",
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
            "- getLocalAABB(): AABB = 0",
            "+ getTransform(): Transform&",
            "+ getMaterial(): Material*",
            "+ getName(): string&",
          )
        ), name: <hittable>),

        node((-1, 2), class(
          "Sphere",
          (
            "# radius: float",
          ),
          (
            "+ Sphere(name, material, radius)",
            "+ hit(ray: const Ray&): HitInfo",
            "+ getRadius(): float&",
            "- getLocalAABB(): AABB",
          )
        ), name: <sphere>),

        node((0, 2), class(
          "Mesh",
          (
            "# tris: vector<RawTriangle>",
            "# bvh_root: unique_ptr<BVHNode>",
            "# aabb: AABB",
          ),
          (
            "+ Mesh(name, material, tris)",
            "+ Mesh(name, material, filename)",
            "+ hit(ray: const Ray&): HitInfo",
            "- getLocalAABB(): AABB",
            "- computeAABB()",
          )
        ), name: <mesh>),

        node((-1, 3.5), class(
          "RawTriangle",
          (
            "# v0, v1, v2: Vec3",
            "# uv0, uv1, uv2: Vec2",
          ),
          (
            "+ RawTriangle(v0, v1, v2)",
            "+ RawTriangle(v0, v1, v2, uv0, uv1, uv2)",
          )
        ), name: <rawtriangle>),

        node((1, 3.5), class(
          "BVHNode",
          (
            "# aabb: AABB",
            "# left, right: unique_ptr<BVHNode>",
            "# start_index, num_objects: int",
          ),
          (
            "+ BVHNode(objects, start, end)",
            "+ isLeaf(): bool",
            "+ traverse(ray, hitFunc): HitInfo",
          )
        ), name: <bvhnode2>),

        edge(<hittable>, <transform>, marks: composition),
        edge(<sphere>, <hittable>, marks: inheritance),
        edge(<mesh>, <hittable>, marks: inheritance),
        edge(<mesh>, <rawtriangle>, marks: composition),
        edge(<mesh>, <bvhnode2>, marks: composition, bend: 12deg),
      )
    ]
  ], caption: [Hệ thống hình học.]
)

Hệ thống hình học được xây dựng quanh lớp cơ sở trừu tượng `Hittable`. Mỗi vật thể lưu một `Transform` (gồm các thông tin về tỉ lệ, góc quay và vị trí) cùng con trỏ tới `Material` là vật liệu của nó. Lớp này khai báo hai phương thức thuần ảo `hit()` và `getLocalAABB()`; các lớp dẫn xuất chỉ cần cài đặt phép giao cắt `hit()` và hàm lấy hình bao `getLocalAABB()` trong không gian cục bộ, còn `getAABB()` sẽ áp dụng `Transform` để trả về hộp bao trong không gian thế giới.

- `Sphere` chỉ lưu bán kính; tâm của nó nằm tại gốc không gian cục bộ `(0, 0, 0)`, còn vị trí trong thế giới được xác định bởi `Transform.translation`.
- `Mesh` quản lý một danh sách `RawTriangle` và một cây BVH (`BVHNode`) để tăng tốc truy vấn giao cắt trên lưới lớn, đồng thời cài đặt thuật toán Moller–Trumbore để kiểm tra giao cắt tia–tam giác và nội suy tọa độ `uv`.
- `RawTriangle` chỉ lưu hình học (các đỉnh và tọa độ kết cấu), không chứa logic giao cắt.

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
            "# roughness: float",
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
            "# emission_strength: float",
            "# color: Color",
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
        edge(<material>, <scatterresult>, marks: dependency),
      )
    ]
  ], caption: [Hệ thống vật liệu.]
)

`Material` định nghĩa giao diện chung cho các loại vật liệu, bao gồm 2 hàm ảo là `scatter()` và `emitted()`. `scatter()` nhận tia hiện tại, màu suy hao bề mặt, `HitInfo` và môi trường hiện tại, rồi tính toán hướng tia mới và trả về một trong ba trạng thái `ScatterResult`: `Terminated` (dừng tia), `Scattered` (tia bị tán xạ trong môi trường hiện tại) hoặc `Refracted` (tia bị khúc xạ qua môi trường khác). `emitted()` lấy vị trí giao điểm `HitInfo` rồi trả về ánh sáng phát ra trên bề mặt vật thể tại đó. Mặc định `Material::scatter()` trả về `Terminated`, còn `Material::emitted()` trả về màu đen `(0, 0, 0)`. `Matte` tán xạ khuếch tán quanh pháp tuyến, `Metal` phản xạ gương có độ nhám, `Dielectric` khúc xạ theo định luật Snell, và `Light` chỉ phát xạ (không tán xạ) và kết thúc tia tại bề mặt.

#figure(
  block(breakable: false, width: 100%)[
    #align(center)[
      #diagram(
        node-shape: rect,
        spacing: (0.2cm, 0.2cm),

        node((-2, 0), class(
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
        ), name: <material>),

        node((1, 0), class(
          "Medium",
          (
            "# name: string",
            "# ior: float",
          ),
          (
            "+ Medium(name, ior)",
            "+ {virtual} transmit(distance): Color",
            "+ getName(): string&",
            "+ getIOR(): float&",
          )
        ), name: <medium>),

        node((-2, 1.6), class(
          "ColorTexture",
          (
            "# color: Color",
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
            "# distance: float",
            "# hit_point: Vec3",
            "# normal: Vec3",
            "# material: const Material*",
            "# front_face: bool",
            "# uv: Vec2",
          ),
          none
        ), name: <hitinfo>),

        node((1, 1.6), class(
          "Homogeneous",
          (
            "# sigma: Color",
            "# density: float",
          ),
          (
            "+ Homogeneous(name, ior, sigma, density)",
            "+ transmit(distance): Color",
            "+ getSigma(): Color&",
            "+ getDensity(): float&",
          )
        ), name: <homogeneous>),

        edge(<material>, <texture>, marks: aggregation),
        edge(<material>, <medium>, marks: aggregation),
        edge(<colortexture>, <texture>, marks: inheritance),
        edge(<homogeneous>, <medium>, marks: inheritance),
        edge(<hitinfo>, <material>, marks: aggregation),
      )
    ]
  ], caption: [Vật liệu, bề mặt và môi trường truyền dẫn.]
)

`Texture` cung cấp dữ liệu bề mặt tại một `HitInfo` bao gồm suy hao bề mặt (màu sắc), độ nhám, pháp tuyến, ...; `ColorTexture` là cài đặt đơn giản nhất, luôn trả về một màu không đổi. `Medium` mô tả môi trường mà tia đang truyền qua. `Medium::transmit(distance)` trả về hệ số suy hao năng lượng trên một đoạn đường dài `distance`; lớp cơ sở trả về màu trắng (không hấp thụ), còn `Homogeneous` cài đặt định luật Beer–Lambert:

$ "transmit"(d) = exp(-sigma dot "density" dot d) $

với phép nhân và hàm mũ được áp dụng cho từng kênh màu. Môi trường toàn cục của cảnh được lưu trong `Scene.global_medium`; mỗi vật liệu `Dielectric` tham chiếu thêm một `Medium` riêng để mô tả môi trường bên trong nó.

#figure(
  block(breakable: false, width: 100%)[
    #align(center)[
      #diagram(
        node-shape: rect,
        spacing: (0.2cm, 0.2cm),

        node((-1, 0), class-sm(
          "Vec2",
          (
            "# x, y: float",
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
            "# x, y, z: float",
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
            "# origin: Vec3",
            "# direction: Vec3",
          ),
          (
            "+ Ray(origin = Vec3(0), direction = Vec3(0, 0, -1))",
            "+ point(distance: float): Vec3",
          )
        ), name: <ray>),

        node((0, 2.8), class-sm(
          "AABB",
          (
            "# minp, maxp: Vec3",
            "# centroid: Vec3",
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
            "# v0, v1, v2: Vec3",
            "# uv0, uv1, uv2: Vec2",
          ),
          (
            "+ RawTriangle(v0, v1, v2)",
            "+ RawTriangle(v0, v1, v2, uv0, uv1, uv2)",
          )
        ), name: <rawtriangle>),

        node((-1, 5.4), class-sm(
          "Transform",
          (
            "# scale: Vec3",
            "# rotation: Mat3x3",
            "# rotation_angles: Vec3",
            "# translation: Vec3",
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
            "# m[3][3]: float",
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
            "# generator: mt19937",
            "# uniform_dist: uniform_real_distribution",
            "# normal_dist: normal_distribution",
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

        edge(<vec2>, <vec3>, marks: dependency),
        edge(<color>, <vec3>, marks: dependency, label: [alias], bend: -10deg),
        edge(<ray>, <vec3>, marks: composition),
        edge(<aabb>, <vec3>, marks: composition),
        edge(<rawtriangle>, <vec3>, marks: composition),
        edge(<transform>, <mat3x3>, marks: composition),
      )
    ]
  ], caption: [Các kiểu toán học hỗ trợ.]
)

Các kiểu toán học được thiết kế dưới dạng `struct` nhỏ, thao tác trên giá trị và không sở hữu tài nguyên. `Vec3` kiêm luôn vai trò `Color` (với các kênh R, G, B lần lược là các trục x, y, z) và cung cấp các phép toán vector, phản xạ, khúc xạ, chuyển đổi màu sang ABGR. `Ray` và `AABB` là hai kiểu nền tảng cho mọi phép truy vấn giao cắt; `RawTriangle` kết hợp `Vec3` và `Vec2` để lưu hình học và tọa độ kết cấu. `Transform` đóng gói phép biến đổi affine, `Mat3x3` dùng để chứa ma trận xoay, còn `RNG` là lớp tiện ích tĩnh phục vụ lấy mẫu ngẫu nhiên trong mô phỏng Monte Carlo.

#pagebreak()

== Cây phân vùng không gian Bounding Volume Hierarchy (BVH)
Thay vì duyệt tuyến tính qua toàn bộ danh sách vật thể để tìm giao điểm, chương trình sử dụng cấu trúc cây BVH để truy vấn giao cắt giữa tia và các vật thể trong cảnh. Cấu trúc của một nút trên cây gồm con trỏ đến 2 nút con (trỏ đến vô định nếu là nút lá), phạm vi vật thể mà nó quản lý và hộp bao AABB của những vật thể đó. Hai nút con của một nút cha sẽ quản lý 2 tập con bù nhau của tập vật thể mà nút cha quản lý.

Khi kiểm tra giao cắt, ta kiểm tra xem tia có cắt với hộp bao của nút BVH không. Nếu có, ta phải lặp lại phép kiểm tra tương tự trên cả 2 nút con của nút còn lại (nếu là nút lá thì tiến hành kiểm tra giao cắt giữa tia - vật thể); nếu không, ta có thể hoàn toàn bỏ qua việc kiểm tra các nút con của nút hiện tại, do đó giảm đáng kể thời gian truy vấn.

Sử dụng cây BVH, ta có thể giảm độ phức tạp thời gian truy vấn từ $O(N)$ đối với tìm kiếm tuần tự xuống trung bình $O(log N)$, với $N$ là số lượng vật thể trong cảnh.
= Thuật toán
Trong các thuật toán, $epsilon$ là mức sai số chấp nhận được dùng trong các phép so sánh số thực.

== Thuật toán giao cắt tia – mặt cầu

=== Đầu vào
- Tia cần kiểm tra có gốc $arrow(o)$ và hướng $arrow(d)$.
- Mặt cầu có bán kính $r$. Giả định tâm hình cầu luôn đặt tại `(0, 0, 0)`.
- Vật liệu của mặt cầu.

=== Đầu ra
- Một cấu trúc `HitInfo` chứa khoảng cách, điểm giao, pháp tuyến, cờ `front_face` và vật liệu của mặt cầu nếu tồn tại giao điểm.

=== Phương pháp
Ta xét bài toán trong không gian cục bộ của mặt cầu, nơi tâm mặt cầu nằm tại gốc tọa độ. Trong phần này, $arrow(o)$ và $arrow(d)$ lần lượt là gốc và hướng của tia trong không gian cục bộ. Điều kiện để một điểm trên tia nằm trên mặt cầu với tham số $t >= 0$ là:

$ ||arrow(o) + t arrow(d)||^2 = r^2 $

Khai triển và thu gọn phương trình bậc hai theo $t$:

$ (arrow(d) dot arrow(d))t^2 + 2(arrow(o) dot arrow(d))t + (arrow(o) dot arrow(o) - r^2) = 0 $

Chương trình sử dụng dạng rút gọn với nửa hệ số bậc nhất để giảm số phép nhân:

$ a = arrow(d) dot arrow(d), quad b_"half" = -(arrow(o) dot arrow(d)), quad c = arrow(o) dot arrow(o) - r^2 $

$ Delta = b_"half"^2 - a c $

Nếu $Delta < 0$, tia không cắt mặt cầu. Ngược lại, hai nghiệm là:

$ t_0 = (b_"half" - sqrt(Delta))/a, quad t_1 = (b_"half" + sqrt(Delta))/a $

Ta thấy rằng $t_0$ luôn bé hơn hoặc bằng $t_1$, và ta cần tìm giao điểm gần nhất cho thuật toán dò tia. Do đó chương trình ưu tiên nghiệm nhỏ hơn $t_0$. Nếu $t_0 < epsilon$, nghĩa là giao điểm nằm phía sau tia hoặc quá gần gốc tia, nghiệm $t_1$ được chọn thay thế. Nếu cả hai nghiệm đều nhỏ hơn $epsilon$, phép kiểm tra thất bại. Bước này vừa tránh hiện tượng tự cắt, vừa xử lý trường hợp gốc tia nằm bên trong mặt cầu.

Sau khi có khoảng cách $t$, điểm giao và pháp tuyến ngoài được tính bởi:

$ arrow(p) = arrow(o) + t arrow(d), quad arrow(n)_"out" = arrow(p)/r $

Cờ `front_face` cho biết tia đang đi vào mặt trước hay mặt sau của mặt cầu:

$ "front_face" = arrow(d) dot arrow(n)_"out" < 0 $

Nếu `front_face` là `true`, pháp tuyến được giữ nguyên; ngược lại, pháp tuyến được đảo dấu để luôn hướng về phía tia tới.

Các công thức nghiệm ở trên dùng trực tiếp $arrow(o)$ và $arrow(d)$ của không gian cục bộ. Khoảng cách thế giới được tính lại ở bước cuối. Để áp dụng cho mặt cầu đã được đặt trong không gian thế giới bởi `Transform` gồm tỉ lệ $S = "diag"(s_x, s_y, s_z)$, ma trận quay $R$ và vector tịnh tiến $arrow(T)$, ta không biến đổi mặt cầu mà đưa tia vào không gian cục bộ của nó:

$ arrow(o)_"local" = S^(-1) R^T (arrow(o)_"world" - arrow(T)), quad arrow(d)_"local" = S^(-1) R^T arrow(d)_"world" $
Lưu ý: vì $R$ là ma trận quay, tức nó là ma trận trực giao, do đó $R^(-1) = R^T$

Phép biến đổi nghịch đảo đưa tia vào không gian cục bộ, nơi mặt cầu vẫn là mặt cầu bán kính $r$. Sau đó chạy thuật toán giao cắt với mặt cầu bán kính $r$ đặt tại gốc tọa độ như bình thường để có được giao điểm $arrow(d)_"local"$ và pháp tuyến $arrow(n)_"local"$ trên không gian cục bộ.

Sau đó các đại lượng được đưa trở lại không gian thế giới:

$ arrow(p)_"world" = arrow(T) + R S arrow(p)_"local", quad arrow(n)_"world" = "normalize"(R S^(-1) arrow(n)_"local") $

Khoảng cách được tính nhanh bằng cách lấy hình chiếu của vector khoảng cách lên hướng tia thế giới:

$ t_"world" = (arrow(p)_"world" - arrow(o)_"world") dot arrow(d)_"world" $

Do $arrow(p)_"word"$, $arrow(d)_"world"$ cùng hướng và $|arrow(d)_"world"| = 1$, $t_"world"$ chính là khoảng cách thực dọc theo tia. Cuối cùng, các giá trị này được đóng gói vào `HitInfo` và trả về.

Độ phức tạp của phép kiểm tra là $O(1)$.

== Thuật toán giao cắt tia – hộp bao AABB

=== Đầu vào
- Tia cần kiểm tra `ray` với gốc `origin` và hướng `direction`.
- Hộp bao `AABB` được biểu diễn bởi hai điểm đối diện `minp` và `maxp`.
- Khoảng tham số hợp lệ $[t_min, t_max]$. Trong quá trình duyệt BVH, $t_min$ thường bằng `EPSILON` và $t_max$ bằng khoảng cách tới va chạm gần nhất hoặc `FAR_DISTANCE`.

=== Đầu ra
- `true` nếu tia cắt hộp bao trong khoảng $[t_min, t_max]$.
- `false` nếu tia không cắt hộp bao, cắt ở phía sau gốc tia, hoặc chỉ cắt ngoài khoảng tham số cho phép.

=== Phương pháp
AABB là giao của ba "slab" song song với các mặt phẳng tọa độ $x$, $y$, $z$. Với mỗi trục $a$, giao điểm của tia với hai mặt phẳng $a = "minp"_a$ và $a = "maxp"_a$ là:

$ t_0 = ("minp"_a - o_a)/d_a, quad t_1 = ("maxp"_a - o_a)/d_a $

Nếu $d_a < 0$, thứ tự hai nghiệm bị đảo nên chương trình hoán đổi $t_0$ và $t_1$. Khoảng giao của tia với slab thứ $a$ là $[t_0, t_1]$. Khoảng giao tổng của tia với cả ba slab là giao của ba khoảng:

$ t_"enter" = max(t_"enter", t_0), quad t_"exit" = min(t_"exit", t_1) $

Ban đầu $t_"enter" = t_min$ và $t_"exit" = t_max$. Sau mỗi trục, nếu $t_"exit" < t_"enter"$ thì khoảng giao rỗng và tia không cắt hộp bao. Nếu duyệt hết cả ba trục mà điều kiện vẫn đúng, tia cắt hộp bao.

Về mặt hình học, $t_"enter"$ là tham số mà tia bắt đầu đi vào hộp, còn $t_"exit"$ là tham số mà tia đi ra khỏi hộp. Nếu $t_"exit" < t_"enter"$, tia đi qua các slab theo thứ tự không hợp lệ, tức là không có đoạn nào nằm đồng thời trong cả ba slab.

```cpp
for (int a = 0; a < 3; ++a) {
    float invD = 1.0f / ray.direction.axis[a];
    float t0 = (minp[a] - ray.origin[a]) * invD;
    float t1 = (maxp[a] - ray.origin[a]) * invD;

    if (invD < 0.0f) std::swap(t0, t1);

    tmin = t0 > tmin ? t0 : tmin;
    tmax = t1 < tmax ? t1 : tmax;

    if (tmax < tmin) return false;
}
return true;
```

Trường hợp $d_a = 0$ được xử lý dựa trên số học dấu phẩy động của IEEE 754: `invD` là $plus.minus infinity$, nên $t_0$ và $t_1$ trở thành các giá trị vô cực. Cách này cho kết quả đúng khi gốc tia không nằm đúng trên mặt slab. Nếu gốc tia nằm chính xác trên mặt slab và $d_a = 0$, phép nhân $0 dot infinity$ có thể tạo ra `NaN`; khi đó cần xử lý riêng nếu muốn đảm bảo ổn định số học tuyệt đối.

Phép kiểm tra AABB là cơ sở để cắt tỉa cây BVH. Trong `BVHNode::traverse`, mỗi nút được kiểm tra bằng `current_node->aabb.hit(ray, EPSILON, t_max)`. Nếu kết quả là `false`, toàn bộ nhánh con của nút bị bỏ qua; nếu là `true`, thuật toán tiếp tục đi xuống nút lá để kiểm tra chi tiết với các hình học bên trong.

Độ phức tạp của một phép kiểm tra AABB là $O(1)$ vì chỉ cần xử lý ba trục.

== Thuật toán giao cắt tia - tam giác (Moller - Trumbore)

=== Đầu vào
- Tia cần kiểm tra với gốc $arrow(o)$ và hướng $arrow(d)$.
- Tam giác với ba đỉnh $arrow(v)_0, arrow(v)_1, arrow(v)_2$ và ba tọa độ kết cấu tương ứng $arrow("uv")_0, arrow("uv")_1, arrow("uv")_2$.
- Khoảng tham số hợp lệ $[t_min, t_max]$.

=== Đầu ra
- `true` nếu tia cắt tam giác trong khoảng tham số cho phép.
- Khoảng cách $t$ và hai tọa độ barycentric $u$, $v$ của giao điểm nếu tồn tại.

=== Phương pháp
Tương tự mặt cầu, các đỉnh tam giác được lưu trong không gian cục bộ của `Mesh`. Thay vì biến đổi từng tam giác, chương trình đưa tia thế giới về không gian cục bộ bằng phép biến đổi nghịch đảo của `Transform` (với $S = "diag"(s_x, s_y, s_z)$, $R$ là ma trận quay và $arrow(T)$ là vector tịnh tiến):

$ arrow(o)_"local" = S^(-1) R^T (arrow(o)_"world" - arrow(T)), quad arrow(d)_"local" = S^(-1) R^T arrow(d)_"world" $

Sau khi tìm được giao điểm gần nhất trong không gian cục bộ, điểm giao và pháp tuyến được đưa trở lại không gian thế giới, còn khoảng cách được tính lại bằng hình chiếu lên tia thế giới:

$ arrow(p)_"world" = arrow(T) + R S arrow(p)_"local", quad arrow(n)_"world" = "normalize"(R S^(-1) arrow(n)_"local"), quad t_"world" = (arrow(p)_"world" - arrow(o)_"world") dot arrow(d)_"world" $

Nhờ đó, thuật toán Moller–Trumbore bên dưới chỉ làm việc với tam giác ở dạng cục bộ, không phụ thuộc vào vị trí, hướng hay tỉ lệ của `Mesh` trong cảnh. Trong phần còn lại của mục, để gọn ký hiệu, $arrow(o)$ và $arrow(d)$ được hiểu là tia trong không gian cục bộ.

Đặt hai cạnh của tam giác:

$ arrow(e)_1 = arrow(v)_1 - arrow(v)_0, quad arrow(e)_2 = arrow(v)_2 - arrow(v)_0 $

Mọi điểm trên tam giác có thể biểu diễn theo tọa độ tỉ cự:

$ arrow(p) = arrow(v)_0 + u arrow(e)_1 + v arrow(e)_2, quad u >= 0, v >= 0, u + v <= 1 $

Giao điểm của tia và tam giác thỏa mãn:

$ arrow(o) + t arrow(d) = arrow(v)_0 + u arrow(e)_1 + v arrow(e)_2, quad t >= 0 $

Đây là hệ ba phương trình tuyến tính với ba ẩn $t$, $u$, $v$. Thuật toán Moller – Trumbore giải hệ này bằng quy tắc Cramer mà không cần tính trực tiếp phương trình mặt phẳng. Các đại lượng trung gian được định nghĩa như sau:

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

rồi xác định `front_face` giống như trường hợp mặt cầu:

$ "front_face" = arrow(d) dot arrow(n)_"out" < 0 $

Nếu `front_face` là `false`, pháp tuyến được đảo dấu. Tọa độ kết cấu tại giao điểm được nội suy tuyến tính với trọng số $w = 1 - u - v$:

$ arrow("uv") = w arrow("uv")_0 + u arrow("uv")_1 + v arrow("uv")_2 $

Sau đó, `Mesh::hit()` áp dụng các phép biến đổi về không gian thế giới như đã trình bày ở đầu mục.

Trong lớp `Mesh`, thuật toán này được gọi cho từng tam giác nằm trong nút lá của BVH. Sau mỗi va chạm, `t_max` được cập nhật bằng khoảng cách gần nhất vừa tìm được, nhờ đó các tam giác ở xa hơn có thể bị loại bỏ sớm.

Độ phức tạp của một phép kiểm tra tia – tam giác là $O(1)$. Khi kết hợp với BVH, số tam giác cần kiểm tra trung bình giảm từ $O(N)$ xuống còn $O(log N)$.

== Thuật toán xây dựng BVH
=== Đầu vào
- Danh sách các vật thể trong khoảng `[start, end)`, với `start` và `end` là chỉ số phần tử.
- Ngưỡng số vật thể tối đa trong một nút lá `PRIM_PER_LEAF`.

=== Đầu ra
- Nút gốc của cây BVH đối với các vật thể từ `start` đến `end` trong danh sách đã cho.

=== Phương pháp
+ Nếu `start >= end`, trả về một nút rỗng (`num_objects = 0`) để tránh truy cập ngoài danh sách.
+ Tính AABB bao trọn toàn bộ vật thể từ `start` đến `end`.
+ Nếu số lượng vật thể `size = end - start` nhỏ hơn hoặc bằng `PRIM_PER_LEAF`, nút hiện tại là nút lá: `left` và `right` bằng `nullptr`, lưu `start_index = start` và `num_objects = size`, rồi kết thúc đệ quy.
+ Ngược lại, chọn trục có chiều dài lớn nhất `axis` của AABB (trục $x$, $y$ hoặc $z$).
+ Sắp xếp các vật thể tăng dần theo tọa độ nhỏ nhất (`minp[axis]`) của AABB tương ứng trên trục vừa chọn.
+ Chia danh sách đã sắp xếp thành hai nửa bằng nhau và gọi đệ quy để tạo `left`, `right`.
+ AABB của nút hiện tại là `span_box` đã tính ở trên; giá trị này tương đương với hợp AABB của `left` và `right`.

Để tạo cây BVH của toàn bộ vật thể trong danh sách `objects`, chỉ cần gọi `BVHNode(objects, 0, objects.size())`.

== Thuật toán truy vấn giao cắt trên cây BVH
=== Đầu vào
- Nút gốc `bvh_root` của cây BVH.
- Tia sáng `ray`.
- Hàm callback `hitFunc(primitive_idx, closest_hit)` để kiểm tra chi tiết từng vật thể trong nút lá.

=== Đầu ra
- `HitInfo` chứa thông tin về điểm giao gần nhất (nếu có); `distance = -1` nếu không cắt.

=== Phương pháp
Chương trình duyệt lặp bằng hàng đợi `std::queue<const BVHNode*>`, không đệ quy:

+ Khởi tạo `node_queue` chứa nút gốc `bvh_root`, `closest_hit` rỗng.
+ Lặp cho đến khi hàng đợi rỗng:
  + Lấy `current_node = node_queue.front()` rồi gọi `node_queue.pop()`.
  + Lấy `t_max = closest_hit.distance`; nếu `closest_hit` rỗng thì `t_max = FAR_DISTANCE` (tức vô cùng).
  + Kiểm tra `current_node->aabb.hit(ray, EPSILON, t_max)`:
    - Nếu không cắt: bỏ qua lượt này.
    - Nếu cắt và là nút nhánh (`left`/`right` khác `nullptr`): thêm `left` và `right` vào hàng đợi.
    - Nếu cắt và là nút lá: gọi `hitFunc` cho từng vật thể trong nút; `closest_hit` được cập nhật sau mỗi va chạm.
+ Trả về `closest_hit`.

Nhờ `t_max` được thu hẹp sau mỗi va chạm, các nút có AABB nằm xa hơn điểm giao gần nhất bị loại bỏ sớm. Độ phức tạp trung bình là $O(log N)$ cho mỗi tia.

== Thuật toán tính hướng khúc xạ
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
Nếu xảy ra phản xạ toàn phần $eta^2 (1 - cos^2 theta) > 1$ hoặc $R > X$ ($X$ là biến ngẫu nhiên phân phối đều từ $0$ đến $1$) và $eta != 1$, tia phản xạ và không thay đổi môi trường. Khi này kết quả là `Scattered`:
$ arrow(d)_"new" = arrow(d) - 2(arrow(d) dot arrow(n))arrow(n) $
Ngược lại, tia khúc xạ qua bề mặt và kết quả là `Refracted`:
$ arrow(d)_"new" = eta arrow(d) + (eta cos theta - sqrt(1 - eta^2 (1 - cos^2 theta))) arrow(n) $

== Thuật toán tính hướng tán xạ
=== Đầu vào
- Hướng tia ban đầu $arrow(d)$.
- Độ nhám vật liệu $r$ có giá trị từ $0$ đến $1$.

=== Đầu ra
- Hướng tia mới $arrow(d_"new")$.
- Kết quả: luôn bị tán xạ `Scattered`.

=== Phương pháp
Với vật liệu kim loại `Metal`, hướng phản xạ gương được tính bởi:
$ arrow(d)_"spec" = arrow(d) - 2(arrow(d) dot arrow(n))arrow(n) $
Khi độ nhám $r > 0$, một vector ngẫu nhiên $arrow(v)_"rand"$ (chưa chuẩn hóa) được nhân với $r$ rồi cộng vào hướng phản xạ:
$ arrow(d)_"new" = "normalize"(arrow(d)_"spec" + r arrow(v)_"rand") $
Nếu $arrow(d)_"new" dot arrow(n) <= 0$, tức hướng mới quay ra sau bề mặt, chương trình phản xạ lại vector này qua $arrow(n)$ trước khi chuẩn hóa.

Vật liệu nhám `Matte` chính là trường hợp đặc biệt của vật liệu kim loại với độ nhám $r = 1$. Do đó thuật toán sinh tia tán xạ của `Matte` về cơ bản là thuật toán sinh tia tán xạ của `Metal` nhưng được tối ưu riêng cho trường hợp này:
$ arrow(d)_"new" = "normalize"(arrow(n) + arrow(v)_"rand") $
trong đó $arrow(v)_"rand"$ là một vector đơn vị ngẫu nhiên.

== Thuật toán dò tia cơ bản.

=== Đầu vào
- Tia sơ cấp `ray`.
- Cấu trúc dữ liệu chứa các vật thể cần kết xuất (cây BVH), cùng môi trường toàn cục `global_medium`.
- Số lần dội tối đa `max_bounces`.

=== Đầu ra
- Màu của tia sau khi dò.

=== Phương pháp
+ *Khởi tạo:* đặt `final_color = (0, 0, 0)`, `throughput = (1, 1, 1)` và một ngăn xếp môi trường `medium_stack` chứa môi trường toàn cục `global_medium`.
+ *Dò tia:* lặp tối đa `max_bounces` lần. Ở mỗi lần dội, thực hiện lần lượt các bước sau:
  + *Kiểm tra giao cắt:* tìm giao điểm gần nhất của `ray` với các vật thể. Nếu tia không cắt vật thể nào thì tính toán ánh sáng nền và thoát khỏi vòng lặp.

  + *Suy hao theo môi trường:* `throughput *= current_medium->transmit(distance)`, với `current_medium` là đỉnh của `medium_stack`. Với môi trường đồng chất `Homogeneous`, hàm `transmit(distance)` chính là định luật Beer–Lambert trên đoạn đường `distance`.

  + *Phát xạ:* `final_color += throughput * material->emitted(rec)`. `Material::emitted` mặc định trả về màu đen; `Light` trả về `texture->get(rec) * emission_strength`.

  + *Tính toán tia mới:* khởi tạo `surface_attenuation = (1, 1, 1)` rồi gọi `material->scatter(ray, surface_attenuation, rec, current_medium)`; hàm này vừa tính toán suy hao bề mặt, vừa tính toán hướng tia mới (dựa trên thuật toán tán xạ/khúc xạ) và trả về kết quả tán xạ/khúc xạ/kết thúc dò tia. Nếu như kết quả là
    - `Terminated`: tia gặp nguồn sáng, thoát khỏi vòng lặp.
    - `Scattered`: tia bị tán xạ. Nhân `throughput *= surface_attenuation` rồi tiếp tục với tia mới
    - `Refracted`: tia bị khúc xạ. Nhân `throughput *= surface_attenuation`; cập nhật `medium_stack`: nếu cắt mặt ngoài của vật liệu thì đẩy môi trường của vật liệu vào; nếu không và đỉnh ngăn xếp đúng là môi trường đó thì lấy ra.
  + *Russian roulette:* khi chỉ số dội `bounces` đủ lớn, đặt `m = max(throughput.x, throughput.y, throughput.z)`. Nếu `RNG::uniform() > m` thì kết thúc tia; ngược lại chia `throughput` cho `m` để bảo toàn năng lượng kỳ vọng. Điều này giúp thiên vị các tia mang ánh sáng mạnh, kết thúc sớm các tia mang ánh sáng yếu nhằm giảm thời gian dò tia cho các tia đem lại hiệu quả thấp.
  + *Lặp lại:* với tia `ray` vừa cập nhật, quá trình tiếp tục cho đến khi tia kết thúc hoặc đạt giới hạn số lần dội.
+ Sau khi thoát khỏi vòng lặp, trả về `final_color` là màu đã dò được từ tia sơ cấp ban đầu.

== Thuật toán kết xuất bằng phương pháp Monte Carlo

=== Đầu vào
- `Camera` gồm vị trí `position`, hệ trục ảnh `(w, u, v)` là các cơ sở ở không gian cục bộ của camera, tiêu cự `focal_length`, khẩu độ `aperture` và độ phơi sáng `exposure`.
- `Scene` chứa cây BVH, vật thể, vật liệu và môi trường toàn cục.
- Kích thước ảnh `W × H`, tương ứng với `viewport_width` và `viewport_height`.
- Số mẫu mỗi pixel `samples_per_pixel` (gọi tắt là `spp`) và số lần dội tối đa `max_bounces`.
- Bộ đệm tích lũy `buffer` và số pass đã kết xuất `pass`.

=== Đầu ra
- Ảnh `pixels` ở dạng ABGR 8-bit, mỗi kênh đã được tone map và hiệu chỉnh gamma.
- `buffer` được cộng dồn để các pass sau tiếp tục hội tụ; `pass` tăng thêm 1 sau mỗi lần kết xuất.

=== Phương pháp
Mỗi pixel nhận ánh sáng từ một tập tia liên tục trên diện tích pixel và trên mặt phẳng khẩu độ. Gọi $I_(i j)$ là màu lý tưởng của pixel ở cột $i$, hàng $j$. Giá trị này là một tích phân trên miền mẫu, nhưng chương trình ước lượng bằng Monte Carlo: lấy $N$ mẫu (với $N$ được định nghĩa ở mục lấy mẫu pixel), tạo một tia cho mỗi mẫu, dò màu rồi lấy trung bình:

$ I_(i j) approx 1/N sum_(k=1)^N L(arrow(r)_k) $

trong đó $L(arrow(r)_k)$ là màu trả về bởi `trace()` cho tia thứ $k$. Vì bản thân `trace()` cũng dùng mẫu ngẫu nhiên cho tán xạ, khúc xạ và Russian roulette, toàn bộ ảnh là một ước lượng Monte Carlo nhiều tầng: lấy mẫu pixel, lấy mẫu khẩu độ và lấy mẫu đường đi ánh sáng. Khi $N$ tăng, phương sai giảm theo $O(1/N)$, nên ảnh hội tụ dần.

==== Lấy mẫu pixel và khử răng cưa
- Xét pixel ở cột `i`, hàng `j`. Chia pixel thành lưới $n times n$, với $n = max(1, floor(sqrt("spp")))$. Số mẫu thực tế mỗi pixel mỗi pass là $N = n^2$; nếu `spp` không phải số chính phương thì $N$ nhỏ hơn giá trị đã đặt.
- Với mỗi ô $(s_x, s_y)$ trong lưới, lấy một điểm ngẫu nhiên bên trong ô:

$ x = (i + (s_x + xi_x)/n)/W - 1/2, quad y = -((j + (s_y + xi_y)/n)/H - 1/2) $

trong đó $xi_x, xi_y$ là hai biến ngẫu nhiên phân phối đều trong $[0, 1)$. Dấu trừ ở $y$ đảo trục tung vì tọa độ ảnh tăng từ trên xuống, còn trục $v$ của camera hướng lên.
- Đặt $"uv" = (x, y)$ và nhân từng thành phần tương ứng với $"world_viewport" = (2 W/H, 2)$ để được tọa độ viewport $"uv_viewport" = (x_v, y_v)$, với $x_v = 2 x W / H$ và $y_v = 2 y$. Sau đó tạo tia bằng `camera.getRayAt(uv_viewport)`. Hướng tia trước khi chuẩn hóa là:

$ arrow(d) = f arrow(w) + x_v arrow(u) + y_v arrow(v) $

trong đó $f$ là `focal_length` và $(arrow(w), arrow(u), arrow(v))$ là các cơ sở của `camera`.
- Vì mỗi mẫu nằm ở một vị trí khác nhau trong pixel, biên vật thể được lấy trung bình theo độ phủ thực để khử răng cưa (anti-aliasing). Cách chia lưới rồi lấy ngẫu nhiên trong từng ô là lấy mẫu phân tầng (stratified sampling), giúp giảm phương sai so với chọn hoàn toàn ngẫu nhiên.

==== Khẩu độ và độ sâu trường ảnh
Khi gọi `Camera::getRayAt()` với `aperture > 0` và `jitter = true`, hàm dịch gốc tia trong mặt phẳng khẩu độ sinh bởi hai vector cơ sở $arrow(u)$ và $arrow(v)$ bằng một offset ngẫu nhiên. Gọi $(Delta u, Delta v)$ = `RNG::pointInCircle(aperture/3)` và gọi $arrow(o)$ là gốc tia trước khi dịch; gốc mới $arrow(o)'$ là:

$ arrow(o)' = arrow(o) + Delta u arrow(u) + Delta v arrow(v) $

Việc lấy nhiều gốc tia khác nhau trong cùng một pixel làm ảnh bị nhòe với các mức khác nhau tùy vào khoảng cách của các vật so với vị trí của `camera`; đây là cơ chế được dùng để xấp xỉ độ sâu trường ảnh (depth of field). Khi `aperture = 0`, gốc tia không dịch và ảnh chỉ còn khử răng cưa.

==== Tích lũy qua nhiều lượt
Gọi `accumulate` là tổng màu của $N$ mẫu trong một pixel ở lượt tích lũy hiện tại; `accumulate / N` là ước lượng của lượt đó. Giá trị này được cộng vào `buffer`.

Gọi $I_p$ là ước lượng màu của lượt tích lũy thứ $p$ và $P$ là giá trị hiện tại của `pass`. Màu trung bình của pixel sau $P + 1$ lượt là:

$ I_P = 1/(P + 1) sum_(p=0)^P I_p $

Vì mỗi lượt dùng một tập mẫu ngẫu nhiên mới từ cùng một hàm phân phối, ảnh dần hội tụ theo định lý giới hạn trung tâm.

==== Hậu kỳ
Gọi `display` là màu trung bình của pixel sau khi chia `buffer` cho số lượt: `display = buffer / (pass + 1)`. Giá trị này vẫn là giá trị màu tuyến tính và có thể lớn hơn 1. Chương trình áp dụng các bước sau theo đúng thứ tự:

- *Tone mapping:* nén dải động (HDR) về không gian hiển thị chuẩn hóa $[0, 1]$ bằng phương pháp phơi sáng:

$ T(c, e) = min(1 - exp(-e c), 1) $

trong đó $c$ là một kênh màu và $e$ là `exposure`. Giá trị âm hoặc `NaN` được đưa về 0 trước khi tính. Hàm này giữ được chi tiết ở vùng sáng thay vì cắt trắng đột ngột.
- *Gamma correction:* mã hóa màu đã tone map sang không gian hiển thị:

$ C_"out" = T(c, e)^(1/2.2) $

Bước này bù cho việc màn hình hiển thị màu theo đường cong gamma xấp xỉ 2.2. Tone mapping phải được thực hiện trước gamma vì tone mapping là phép phi tuyến trên giá trị màu tuyến tính.
- *Lượng tử hóa:* mỗi kênh được kẹp vào $[0, 1]$, nhân với 255, cộng 0.5 rồi lấy phần nguyên để chuyển thành số nguyên 8-bit. `toABGR()` đóng gói ba kênh màu và alpha thành một số nguyên 32-bit (kênh alpha được thêm vào để tương thích với các thư viện đồ họa và được gán bằng 255).

==== Mã giả
```cpp
int n = max(1, (int)sqrt(spp));
int N = n * n;
Vec2 world_viewport = Vec2(2.0f * W / H, 2.0f);

for (each pass) {
    float exposure = camera.getExposure();

    for (each pixel (i, j)) {
        Color accumulate = Color(0);

        for (each stratified sample) {
            Vec2 uv = ...;  // pixel coordinates in [-0.5, 0.5]
            Vec2 uv_viewport = uv * world_viewport;
            Ray ray = camera.getRayAt(uv_viewport);
            accumulate += trace(ray, scene);
        }

        buffer[i, j] += accumulate / N;

        Color display = buffer[i, j] / (pass + 1);
        display = gammaCorrect(exposureTonemap(display, exposure), 2.2f);
        pixels[i, j] = display.toABGR();
    }

    pass++;
}
```

Độ phức tạp của một pass là $O(W H N C)$, với $C$ là chi phí trung bình của một lần `trace()`. Chi phí này phụ thuộc vào `max_bounces`, cấu trúc BVH và số vật thể trong cảnh.

Tóm lại, `RayTracer::render()` kết hợp lấy mẫu phân tầng trên pixel, offset khẩu độ, tích lũy nhiều pass và hậu kỳ tone mapping/gamma để biến các ước lượng Monte Carlo còn nhiễu thành ảnh hiển thị ổn định. Số mẫu mỗi pixel và số pass càng lớn thì ảnh càng hội tụ, đổi lại thời gian kết xuất tăng tuyến tính.
