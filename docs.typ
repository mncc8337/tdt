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

= Đồ án pbl2: cơ sở lập trình

== Lời mở đầu
=== Lý do chọn đề tài
Từ phim ảnh, trò chơi điện tử cho đến kiến trúc và thiết kế sản phẩm, đồ họa máy tính ngày càng khẳng định vai trò cốt lõi. Nếu phương pháp rasterization được ưu tiên trong các ứng dụng thời gian thực nhờ tốc độ xử lý vượt trội — nhưng phải đánh đổi bằng việc kết hợp nhiều kỹ thuật rời rạc để xấp xỉ bóng đổ, phản xạ hay khúc xạ — thì *Ray Tracing* (dò tia) lại chọn cách mô phỏng trực tiếp đường đi của ánh sáng. Nhờ đó, mọi hiệu ứng quang học đều xuất hiện một cách hoàn toàn tự nhiên từ một nguyên lý toán học thống nhất, mang lại chất lượng hình ảnh chân thực và sống động hơn.

Không dừng lại ở đó, đề tài này còn là dịp tuyệt vời để vận dụng thực tế hàng loạt kiến thức chuyên sâu của chương trình học: từ tư duy lập trình hướng đối tượng (tính kế thừa, đa hình,...), các cấu trúc dữ liệu linh hoạt (cây nhị phân, ngăn xếp, mảng động), những thuật toán tối ưu (chia để trị, sắp xếp, đệ quy) cho đến nền tảng toán học vững chắc (vector, ma trận, phương trình bậc hai,...). Kết quả của chương trình là hình ảnh quan sát được trực tiếp, giúp việc kiểm tra tính đúng đắn của từng thành phần trở nên trực quan.
=== Mục tiêu của đề tài
Đề tài hướng tới việc hiện thực hóa một hệ thống dựng hình 3D hoàn chỉnh dựa trên thuật toán Ray Tracing từ con số không, không sử dụng engine hay thư viện dựng hình 3D có sẵn (chỉ dùng SFML và ImGui cho giao diện, OpenMP cho đa luồng và các thư viện tiêu chuẩn `std` của C++), nhằm nắm vững bản chất cốt lõi của đồ họa máy tính và các định luật quan học. Cụ thể, các mục tiêu chính bao gồm:

+ Xây dựng hệ thống cốt lõi (Core Engine)
  - Tự hiện thực hóa toàn bộ cấu trúc toán học nền tảng (vector, ma trận, phương trình giao cắt tia - hình học).
  - Thiết kế kiến trúc phần mềm theo định hướng hướng đối tượng (OOP) linh hoạt, có tính đóng gói cao và hỗ trợ mở rộng các loại hình học và vật liệu mới.

+ Hỗ trợ hình học và quang học vật liệu
  - *Hình học:* Xử lý chính xác giao cắt tia với các nguyên hàm cơ bản (hình cầu) và lưới đa giác phức tạp (tam giác, mô hình mesh 3D).
  - *Vật liệu:* Mô phỏng các bề mặt khác nhau bao gồm vật liệu nhám (diffuse), gương phản xạ hoàn hảo (specular/mirror) và vật liệu khúc xạ trong suốt (dielectric/glass).

+ Tối ưu hóa hiệu năng và chất lượng hình ảnh
  - *Tối ưu hóa:* Ứng dụng cấu trúc phân cấp thể tích bao (Bounding Volume Hierarchy - BVH) kết hợp thuật toán phân chia không gian để tăng tốc độ dò tia, giải quyết bài toán nút thắt cổ chai về hiệu năng của Ray Tracing.
  - *Khử nhiễu và lấy mẫu:* Sử dụng phương pháp lấy mẫu Monte Carlo để mô phỏng ánh sáng gián tiếp, bóng đổ mềm, độ sâu trường ảnh (Depth of Field, ở mức xấp xỉ) và tích lũy mẫu qua nhiều lượt để giảm dần nhiễu theo thời gian. Chương trình chưa có bộ khử nhiễu (Denoising) riêng.
=== Phạm vi nghiên cứu
Đề tài tập trung hiện thực hóa một hệ thống dựng hình lũy tiến chạy hoàn toàn trên CPU, có giao diện tương tác (SFML + ImGui) hiển thị ảnh đang hội tụ theo thời gian, với các giới hạn và định hướng kỹ thuật cụ thể như sau:

*Hình học đối tượng:*
  - Hỗ trợ hai dạng hình học cốt lõi: các nguyên hàm mặt cầu (sphere) và lưới đa giác tam giác (triangle mesh) được phân tích cú pháp trực tiếp từ tệp định dạng chuẩn .obj.

*Hệ thống vật liệu và quang học:*
  - Tích hợp 4 mô hình vật liệu tiêu chuẩn gồm: khuếch tán nhám (Matte/Lambertian), kim loại phản xạ (Metal), điện môi khúc xạ trong suốt (Dielectric/Glass) và nguồn sáng phát quang trực tiếp (Light).
  - Hỗ trợ kết cấu màu sắc đơn sắc (solid color/textures) và môi trường không gian đồng nhất có tính chất hấp thụ ánh sáng.

*Phương pháp dựng hình và lấy mẫu:*
  - Ứng dụng cơ chế dựng hình tích lũy theo từng lượt. Trong mỗi lượt, hệ thống bổ sung một mẫu cho mỗi điểm ảnh và hiển thị giá trị trung bình tích lũy theo thời gian thực, giúp người dùng quan sát trực quan quá trình khử nhiễu hội tụ của thuật toán Monte Carlo.
=== Phương pháp thực hiện
+ Nghiên cứu lý thuyết.
    - Tìm hiểu nguyên lý hoạt động của kỹ thuật Ray Tracing, quá trình truyền tia sáng và cách xác định giao điểm giữa tia với các đối tượng trong không gian 3D.
    - Nghiên cứu các mô hình chiếu sáng và tương tác ánh sáng với vật liệu, bao gồm phản xạ khuếch tán, phản xạ kim loại, khúc xạ và hấp thụ ánh sáng.
    - Tìm hiểu phương pháp lấy mẫu Monte Carlo, cơ chế dựng hình tích lũy (Progressive Rendering) và cấu trúc phân cấp thể tích bao (Bounding Volume Hierarchy – BVH) nhằm cải thiện hiệu năng dò tia.
    - Tài liệu đã tham khảo:
        - [RayTracing In One Weekend](https://raytracing.github.io/)
        - [Ray tracing (graphics)-Wikipedia](https://en.wikipedia.org/wiki/Ray_tracing_(graphics))
+ Phân tích bài toán.
    - Xác định các chức năng của hệ thống: xây dựng cảnh 3D, tạo và truy vết tia sáng, xác định giao cắt với vật thể, mô phỏng tương tác ánh sáng và hiển thị hình ảnh kết quả.
    - Phân tích thành phần cần thiết để mô phỏng một cảnh 3D, bao gồm hình học, camera, vật liệu, nguồn sáng và môi trường.
    - Đề xuất sử dụng BVH để giảm số lượng phép kiểm tra giao cắt không cần thiết, kết hợp với xử lý đa luồng và cơ chế tích lũy mẫu để cải thiện hiệu quả dựng hình.
+ Thiết kế cấu trúc dữ liệu.
    - Xây dựng các cấu trúc nền tảng như vector 2D, vector 3D, ...
    - Thiết kế các lớp biểu diễn tia sáng, thông tin giao cắt, hộp bao AABB và các đối tượng hình học
    - Xây dựng hệ thống vật liệu theo hướng đối tượng, cho phép mô phỏng những đặc tính quang học khác nhau.
    - Thiết kế cấu trúc BVH để tổ chức các đối tượng trong không gian, hỗ trợ tăng tốc quá trình tìm kiếm giao điểm.
    - Xây dựng quy trình Path Tracing kết hợp lấy mẫu Monte Carlo và tích lũy kết quả qua nhiều lượt dựng hình
+ Cài đặt và kiểm thử.
    - Sử dụng ngôn ngữ C++ để cài đặt các cấu trúc dữ liệu, thuật toán và các thành phần cốt lõi của hệ thống theo định hướng lập trình hướng đối tượng (OOP).
    - Tiến hành kiểm thử từng thành phần, bao gồm phép giao cắt tia với mặt cầu và tam giác, quá trình đọc mô hình từ tệp OBJ, hoạt động của BVH và khả năng mô phỏng các loại vật liệu.
    - Kiểm tra kết quả dựng hình thông qua các cảnh thử nghiệm, đồng thời đánh giá chất lượng hình ảnh, tốc độ xử lý và khả năng tương tác của chương trình.
=== Lời cảm ơn
.....Thanks.....abcxyz

== Giới thiệu tổng quan đề tài
=== Giới thiệu về kỹ thuật Ray Tracing
Ray Tracing (kỹ thuật dò tia) là một phương pháp dựng hình đồ họa máy tính dựa trên việc mô phỏng quá trình truyền và tương tác của ánh sáng với các vật thể trong không gian ba chiều. Phương pháp này cho phép tạo ra những hình ảnh có độ chân thực cao thông qua việc tính toán các hiện tượng quang học như phản xạ, khúc xạ, đổ bóng và tương tác giữa ánh sáng với vật liệu. Ray Tracing nổi bật ở khả năng mô phỏng các hiện tượng quang học một cách chính xác và chân thực.
#figure(
  image("https://thumb.wikimedia.org/wikipedia/commons/thumb/e/ec/Glasses_800_edit.png/960px-Glasses_800_edit.png?utm_source=en.wikipedia.org&utm_campaign=parser&utm_content=thumbnail", width: 80%),
  caption: [Ray tracing can create photorealistic images.],
  supplement: none,
)
=== Nguyên lý hoạt động tổng quát của Ray Tracing
==== Dò tia ngược (backward tracing)
Trong thực tế, ánh sáng phát ra từ nguồn sáng có thể truyền theo nhiều hướng khác nhau, nhưng chỉ một phần rất nhỏ ánh sáng phản xạ hoặc phát ra từ vật thể đi vào mắt người quan sát. Vì vậy, nếu mô phỏng toàn bộ tia sáng phát ra từ nguồn sáng thì sẽ gây lãng phí đáng kể tài nguyên tính toán.

Để giải quyết vấn đề này, Ray Tracing sử dụng phương pháp dò tia ngược, hay còn gọi là dò tia từ camera.

Thay vì bắt đầu từ nguồn sáng, thuật toán thực hiện các bước:
+ Xác định vị trí camera trong không gian ba chiều.
+ Tạo một mặt phẳng ảnh (Image Plane) trước camera, tương ứng với màn hình hiển thị.
+ Với mỗi pixel trên mặt phẳng ảnh, tạo một tia sáng xuất phát từ camera đi qua pixel đó vào không gian.
+ Kiểm tra sự giao nhau giữa tia sáng và các vật thể trong cảnh.
+ Tính toán màu sắc của pixel dựa trên vật liệu và các tương tác ánh sáng tại điểm giao.

#figure(
  image("https://github-anderson.github.io/lessons/GAMES101/img/ray.png", width: 80%),
  caption: [Minh họa nguyên lý dò tia ngược trong Ray Tracing],
  supplement: none,
)

Phương pháp này tập trung tính toán những tia sáng có khả năng đóng góp trực tiếp vào hình ảnh cuối cùng, nhờ đó giảm đáng kể số lượng phép tính không cần thiết.
==== Các bước hoạt động của thuật toán
Quá trình dựng hình bằng kỹ thuật Ray Tracing có thể được chia thành các bước chính sau:

*Bước 1: Khởi tạo tia sáng (Ray Generation)*

Từ vị trí camera, chương trình tạo ra các tia sáng đi qua từng pixel trên mặt phẳng ảnh. Mỗi tia được biểu diễn bởi phương trình:

$ P(t) = O + t D $

Trong đó:
- $O$: Điểm xuất phát của tia.
- $D$: Vector hướng của tia.
- $t$: Tham số xác định vị trí trên tia.
- $P(t)$: Điểm bất kỳ nằm trên tia.

*Bước 2: Tìm giao điểm gần nhất (Ray Intersection)*
Kiểm tra sự giao nhau giữa tia sáng và các vật thể trong cảnh. Nếu tia cắt nhiều vật thể, chương trình lựa chọn giao điểm gần camera nhất để xác định vật thể mà tia sáng thực sự gặp đầu tiên.

*Bước 3: Tính toán màu sắc (Shading)*
Sau khi xác định giao điểm, chương trình tính toán màu sắc dựa trên các thuộc tính của vật liệu như màu nền, độ phản xạ và độ trong suốt. Các yếu tố ánh sáng cũng được xem xét để tạo ra màu sắc cuối cùng cho pixel.

*Bước 4: Xử lý phản xạ và khúc xạ (Reflection and Refraction)*
Tại giao điểm, chương trình có thể tạo ra các tia mới tương ứng với hiện tượng phản xạ hoặc khúc xạ. Những tia này tiếp tục được truy vết để xác định các tương tác ánh sáng tiếp theo.
Quá trình có thể được thực hiện đệ quy và giới hạn bằng độ sâu truy vết tối đa nhằm kiểm soát chi phí tính toán.

*Bước 5: Xuất hình ảnh (Image Output)*
Sau khi hoàn thành quá trình tính toán cho tất cả pixel, chương trình tổng hợp kết quả thành một hình ảnh hoàn chỉnh và hiển thị trong cửa sổ ứng dụng (chưa hỗ trợ xuất ra tệp ảnh).
=== Mục tiêu và chức năng của chương trình
*Dữ liệu đầu vào (Input)* là cảnh được khai báo trong mã nguồn (main.cpp); sau khi chạy, các thuộc tính của camera, vật thể, vật liệu, kết cấu và môi trường có thể chỉnh trực tiếp qua giao diện. Cảnh gồm:
- Camera: Thiết lập vị trí đặt mắt, điểm nhìn (look-at), tiêu cự (khoảng cách tới mặt phẳng ảnh, quyết định góc nhìn), khẩu độ (cho hiệu ứng Depth of Field xấp xỉ), độ mạnh khử răng cưa và độ phơi sáng. Vector hướng lên được cố định là (0, 1, 0).
- Vật thể & Hình học: Các nguyên hàm hình học cơ bản (mặt cầu) và các mô hình lưới đa giác (triangle mesh) phức tạp được nạp trực tiếp từ các tệp định dạng chuẩn .obj.
- Vật liệu & Kết cấu (Materials & Textures): Khởi tạo các thông số quang học cho 4 nhóm vật liệu (Matte, Metal, Dielectric, Light), màu sắc đơn sắc (SolidTexture). Chưa hỗ trợ bản đồ kết cấu từ ảnh.
- Môi trường (Environment): Định nghĩa môi trường đồng nhất có chiết suất và tính chất hấp thụ ánh sáng (định luật Beer–Lambert); chưa mô phỏng tán xạ thể tích.
Cảnh mẫu chuẩn (Cornell Box Scene): Thiết lập không gian buồng Cornell kinh điển gồm có sàn nhà, trần nhà, ba bức tường (trái, phải, sau), một nguồn sáng dạng diện tích (area light) đặt trên trần, và hai khối hộp đặt bên trong phòng với các góc xoay khác nhau để kiểm tra hiện tượng khử bóng đổ và phản xạ khuếch tán (color bleeding).

#figure(
  image("https://thumb.wikimedia.org/wikipedia/commons/thumb/9/92/Cornell_Box_Octane_%286K%2C_8bit%29.png/330px-Cornell_Box_Octane_%286K%2C_8bit%29.png?utm_source=en.wikipedia.org&utm_campaign=parser&utm_content=thumbnail", width: 80%),
  caption: [inserwt caption hehre],
  supplement: none,
)

*Dữ liệu đầu ra (Output)*
- Hiển thị trực quan thời gian thực: Hình ảnh dựng hình được render trên một luồng riêng và hiển thị động trong cửa sổ ứng dụng đồ họa. Chương trình chưa hỗ trợ xuất ảnh ra tệp.
- Giao diện tương tác (ImGui): chọn vật thể bằng cách nhấp chuột lên khung hình; chỉnh vị trí, góc quay, tỉ lệ, bán kính, màu sắc, chiết suất, độ nhám, cường độ sáng, mật độ hấp thụ; chỉnh vị trí, điểm nhìn, tiêu cự, khẩu độ, khử răng cưa và phơi sáng của camera. Mỗi thay đổi đặt lại quá trình tích lũy.
- Cơ chế dựng hình lũy tiến (Progressive Rendering): Chất lượng hình ảnh và độ chi tiết tăng dần theo số lượt (pass) dựng hình. Mỗi lượt bổ sung thêm các mẫu Monte Carlo cho từng điểm ảnh, giúp khử nhiễu dần theo thời gian thực để người dùng quan sát trực quan quá trình hội tụ của thuật toán Ray Tracing.

*Các thành phần và chức năng chính của hệ thống*

| Nhóm | Chức năng | Mô tả |
|---|---|---|
| *Hình học* | Mặt cầu, lưới tam giác | Mặt cầu được giải bằng phương trình bậc hai; lưới tam giác được đọc từ tệp `.obj` và kiểm tra giao cắt bằng thuật toán Möller–Trumbore. |
| *Biến đổi* | Dịch chuyển, quay, co giãn | Áp dụng các phép biến đổi cho mọi vật thể thông qua lớp `Transform`. |
| *Vật liệu* | Matte | Mô phỏng tán xạ khuếch tán theo mô hình Lambertian. |
|| Metal | Mô phỏng phản xạ gương với tham số điều chỉnh độ nhám. |
|| Dielectric | Mô phỏng hiện tượng khúc xạ theo định luật Snell và phản xạ theo xấp xỉ Schlick. |
|| Light | Mô phỏng nguồn sáng diện tích với cường độ có thể điều chỉnh. |
| *Môi trường* | Homogeneous | Mô phỏng môi trường đồng nhất, hỗ trợ hấp thụ ánh sáng theo khoảng cách dựa trên định luật Beer–Lambert và chiết suất. |
| *Camera* | Tiêu cự, vị trí, hướng nhìn | Thiết lập các thông số quan sát của camera, bao gồm tiêu cự, vị trí và hướng nhìn. |
|| Độ sâu trường ảnh, khử răng cưa, phơi sáng | Hỗ trợ điều chỉnh khẩu độ (xấp xỉ độ sâu trường ảnh bằng cách dịch gốc tia), khử răng cưa (Anti-aliasing) bằng jitter hướng tia và phơi sáng (Exposure). |
| *Path Tracing* | Dò tia toàn cục | Mô phỏng quá trình truyền ánh sáng trong toàn bộ cảnh thông qua việc truy vết các tia sáng. |
|| Kiểm soát quá trình truy vết | Hỗ trợ tối đa 50 lần nảy tia (bounces), sử dụng Russian Roulette và tích lũy kết quả qua nhiều lượt lấy mẫu. |
| *Tăng tốc* | BVH hai cấp, OpenMP | Sử dụng cấu trúc BVH hai cấp để tăng tốc tìm kiếm giao điểm, kết hợp OpenMP để xử lý song song. |
|| Tối ưu giao cắt và dựng ảnh | BVH được xây cho tập vật thể của cảnh và cho tập tam giác của từng mesh, được làm phẳng thành mảng và duyệt bằng ngăn xếp cố định; vòng lặp điểm ảnh chạy song song bằng OpenMP. |
| *Giao diện* | SFML + ImGui | Luồng kết xuất và luồng giao diện tách riêng; chỉnh sửa từ giao diện được xếp hàng và áp dụng khi lấy được khóa cảnh. |

== cơ sở lý thuyết
=== ý tưởng
Chương trình xác định màu sắc của mỗi điểm ảnh (pixel) thông qua quy trình lặp dò tia theo quỹ đạo ánh sáng ngược. Tại mỗi giao điểm nảy tia, hệ thống thực hiện kiểm tra giao cắt giữa tia sáng và cảnh 3D. Lượng ánh sáng tự phát xạ (emitted light) từ bề mặt đối tượng sẽ được tích lũy vào kết quả màu tổng cộng sau khi nhân với hệ số suy hao năng lượng tích lũy (throughput). Tiếp đó, tính chất vật liệu tại giao điểm sẽ quyết định hướng tán xạ (phản xạ hoặc khúc xạ) và mức độ suy hao năng lượng cho lượt nảy kế tiếp của tia sáng.

Dưới đây là mã giả thể hiện thuật toán tính toán năng lượng ánh sáng dọc theo quỹ đạo tia:
```text
Color trace(ray):
    color = (0, 0, 0)
    throughput = (1, 1, 1)
    medium_stack = [môi_trường_toàn_cục]
    lặp tối đa 50 lần:
        hit = tìm giao điểm gần nhất của ray với cảnh
        nếu không có giao điểm: dừng        (nền đen)
        throughput *= medium_stack.top.transmit(hit.distance)
        color += throughput * vật_liệu.emitted(hit)
        kết_quả = vật_liệu.scatter(ray, hit)
        nếu kết_quả là Terminated: dừng
        throughput *= suy_hao_của_bề_mặt
        nếu kết_quả là Refracted: đẩy/lấy môi trường của vật liệu khỏi medium_stack
        từ lần nảy thứ 5: áp dụng Russian roulette
    trả về color
```

#figure(
  image("https://thumb.wikimedia.org/wikipedia/commons/thumb/d/dc/Ray_trace_diagram_vi.svg/960px-Ray_trace_diagram_vi.svg.png?utm_source=vi.wikipedia.org&utm_campaign=parser&utm_content=thumbnail", width: 80%),
  caption: [Giải thuật dò tia kiến tạo một bức ảnh bằng cách bắn các tia vào một cảnh],
  supplement: none,
)

=== cơ sở lý thuyết
==== phương trình giao cắt tia - mặt cầu
Một tia được biểu diễn dưới dạng phương trình tham số:
$ P(t) = O + t D $
với $O$ là gốc tia, $D$ là hướng tia (vector đơn vị), và $t$ là khoảng cách dọc theo tia ($t > 0$).

Xét mặt cầu có tâm $C$ và bán kính $r$, tập hợp các điểm thuộc mặt cầu thỏa mãn phương trình hình học:
$ |P - C|^2 = r^2 $

Thay phương trình tham số của tia $P(t)$ vào phương trình mặt cầu và đặt $m = O - C$, ta thu được phương trình bậc hai theo ẩn số thời gian $t$:
$ (D dot D)t^2 + 2(m dot D)t + (m dot m - r^2) = 0 $

Để tối ưu hóa hiệu năng tính toán và giảm thiểu số phép nhân trong vòng lặp dò tia, chương trình sử dụng dạng rút gọn thông qua hệ số bậc nhất phân nửa ($h = -m dot D$):
$ a = D dot D, quad h = -(m dot D), quad c = m dot m - r^2, quad Delta = h^2 - a dot c $

Nếu $Delta < 0$: Tia không cắt mặt cầu. Nếu $Delta >= 0$: Tồn tại hai nghiệm giao cắt:
$ t_0 = (h - sqrt(Delta))/a quad "và" quad t_1 = (h + sqrt(Delta))/a $

Hệ thống ưu tiên chọn nghiệm $t_0$. Nếu $t_0 < epsilon$ (nghiệm nằm phía sau gốc tia hoặc quá sát gốc do sai số số thực), thuật toán sẽ chuyển sang xét nghiệm $t_1$. Nếu cả hai nghiệm đều nhỏ hơn ngưỡng an toàn $epsilon$, tia được xác định là không giao cắt.

*Xác định pháp tuyến bề mặt:*

Pháp tuyến ngoài tại điểm giao cắt $P$ được tính bằng:
$ N = (P - C)/r $

Để đảm bảo pháp tuyến luôn hướng về phía đối diện với tia tới (phục vụ tính toán chiếu sáng và khúc xạ chính xác), chương trình kiểm tra tích vô hướng $D dot N$. Nếu $D dot N < 0$, tia đang đi vào từ mặt ngoài (front_face = true); ngược lại, pháp tuyến sẽ bị đảo dấu ($"normal" = -N$) để luôn hướng về phía bán cầu đón tia.

*Hỗ trợ phép biến đổi không gian(Transformation)*:

Để hỗ trợ các phép dịch chuyển (translation), quay (rotation) và co giãn (scaling), quá trình kiểm tra giao cắt được thực hiện trong không gian cục bộ (local space) của vật thể:

1. Gốc và hướng tia được chuyển đổi về không gian cục bộ thông qua ma trận nghịch đảo ($"Inverse Transform"$), khi đó mặt cầu có tâm đặt tại gốc tọa độ.
2. Do phép co giãn làm cho vector hướng tia cục bộ không còn giữ tính chất vector đơn vị, hệ số $a = D dot D$ được tính toán trực tiếp thay vì gán cứng bằng $1$.
3. Sau khi tìm được điểm giao và pháp tuyến trong không gian cục bộ, kết quả được ánh xạ ngược trở lại không gian thế giới (world space), đồng thời khoảng cách thực tế được tính lại theo công thức $t = (P - O) dot D$ với $D$ là hướng tia gốc trong không gian thế giới.
==== phương trình giao cắt tia - tam giác
Mọi điểm nằm bên trong một tam giác với ba đỉnh $v_0, v_1, v_2$ đều có thể được biểu diễn duy nhất thông qua tọa độ trọng tâm (barycentric coordinates). Đặt hai vector cạnh xuất phát từ đỉnh $v_0$ là:
$ e_1 = v_1 - v_0 quad "và" quad e_2 = v_2 - v_0 $

Một điểm bất kỳ $P$ trên mặt phẳng tam giác được xác định theo phương trình tham số:
$ P(u, v) = v_0 + u dot e_1 + v dot e_2 $

với các điều kiện ràng buộc biên: $u >= 0$, $v >= 0$ và $u + v <= 1$.

Khi xét giao cắt giữa tia $P(t) = O + t dot D$ và tam giác, ta thiết lập phương trình cân bằng:
$ O + t dot D = v_0 + u dot e_1 + v dot e_2 $

Đây là một hệ ba phương trình tuyến tính với ba ẩn số thực $(t, u, v)$. Để giải quyết hệ phương trình này một cách tối ưu mà không cần tường minh hóa phương trình mặt phẳng chứa tam giác, chương trình sử dụng thuật toán Möller–Trumbore dựa trên quy tắc Cramer:
1. Tính toán vector phụ trợ $"pvec"$ và định thức (det):
    $ "pvec" = D times e_2, quad "det" = e_1 dot "pvec" $
    - Nếu $|"det"| < epsilon$: Tia song song với mặt phẳng chứa tam giác, không xảy ra giao cắt.
2. Tính tọa độ trọng tâm $u$:
    $ "tvec" = O - v_0, quad u = ("tvec" dot "pvec")/"det" $
    - Loại bỏ giao điểm nếu $u < 0$ hoặc $u > 1$.
3. Tính tọa độ trọng tâm $v$:
    $ "qvec" = "tvec" times e_1, quad v = (D dot "qvec")/"det" $
    - Loại bỏ giao điểm nếu $v < 0$ hoặc $u + v > 1$.
4. Xác định khoảng cách $t$:
    $ t = (e_2 dot "qvec")/"det" $
    - Chấp nhận giao điểm nếu thỏa mãn điều kiện khoảng cách $epsilon <= t <= t_max$.
==== phương trình giao cắt tia - hình bao
Trong các mô hình 3D thực tế, một lưới tam giác (mesh) có thể chứa hàng chục hoặc hàng trăm nghìn tam giác. Nếu thực hiện kiểm tra giao cắt tuần tự giữa tia sáng và từng tam giác, chi phí tính toán sẽ tăng tuyến tính $cal(O)(N)$, dẫn đến suy giảm nghiêm trọng hiệu năng dựng hình.

Để giải quyết bài toán nút thắt cổ chai này, chương trình áp dụng kỹ thuật phân chia không gian thông qua cấu trúc Hộp bao song song các trục tọa độ (Axis-Aligned Bounding Box - AABB) kết hợp với cây phân cấp thể tích Bounding Volume Hierarchy (BVH). Ý tưởng cốt lõi là bao bọc một cụm tam giác bên trong một hộp bao hình học đơn giản. Nếu tia không cắt hộp bao, toàn bộ nhóm tam giác bên trong sẽ bị loại bỏ ngay lập tức mà không cần kiểm tra chi tiết từng mặt phẳng tam giác. Trong chương trình, cây được xây đệ quy (sắp xếp theo trục dài nhất của hộp bao rồi chia đôi, mỗi nút lá chứa tối đa 4 đối tượng), sau đó được làm phẳng thành mảng và duyệt bằng ngăn xếp cố định, không đệ quy. Cây được dùng ở hai cấp: cấp cảnh (các vật thể) và cấp mesh (các tam giác).

*Kiểm tra giao cắt Tia – Hộp bao AABB (Slab Method)*

Mỗi hộp bao AABB được xác định duy nhất bởi hai điểm cực trị trong không gian: tọa độ tối thiểu $"minp"$ và tọa độ tối đa $"maxp"$.

Về mặt hình học, một hộp AABB được xem là giao của ba khoảng không gian song song gọi là các slab (tương ứng với ba trục tọa độ $x, y, z$). Đối với mỗi trục $a in {x, y, z}$, thời điểm tia $P(t) = O + t dot D$ cắt hai mặt phẳng giới hạn của slab được tính toán như sau:
$ t_0 = ("minp"[a] - O[a])/(D[a]), quad t_1 = ("maxp"[a] - O[a])/(D[a]) $

Nếu hướng tia có thành phần âm ($D[a] < 0$), vị trí giao cắt gần và xa sẽ bị đảo ngược, do đó chương trình thực hiện hoán đổi giá trị $t_0$ và $t_1$ để đảm bảo khoảng thời gian tia di chuyển qua slab luôn là $[t_0, t_1]$.

Khoảng thời gian tia nằm bên trong đồng thời cả ba slab chính là phép giao của ba khoảng đơn lẻ, được cập nhật tuần tự qua các bước:
$ t_"in" = max(t_"in", t_0), quad t_"out" = min(t_"out", t_1) $

- Ban đầu, thiết lập giới hạn tìm kiếm $t_"in" = t_min$ và $t_"out" = t_max$.
- Nếu tại bất kỳ trục nào mà khoảng thời gian thỏa mãn vi phạm điều kiện $t_"out" < t_"in"$, khoảng giao bị rỗng, nghĩa là tia không cắt hộp bao và thuật toán dừng kiểm tra sớm.
- Nếu duyệt qua cả ba trục mà điều kiện vẫn hợp lệ, tia được xác định là đã cắt hộp AABB.
==== các định luật về tán xạ ánh sáng
Khi tia chạm vào bề mặt, hướng đi tiếp theo phụ thuộc vào vật liệu. Chương trình mô phỏng ba cơ chế: phản xạ gương, khúc xạ và tán xạ khuếch tán.
===== cơ chế phản xạ bề mặt gương
Theo định luật phản xạ quang học, góc phản xạ luôn bằng góc tới và tia phản xạ đồng phẳng với tia tới cùng pháp tuyến bề mặt. Cho vector hướng tia tới $D$ và pháp tuyến đơn vị $N$ tại điểm giao cắt, vector phản xạ hoàn hảo $R$ được xác định bởi công thức:
$ R = D - 2(D dot N)N $

Về mặt hình học, thành phần của tia $D$ song song với $N$ bị đảo dấu, trong khi thành phần vuông góc được giữ nguyên.

Đối với các bề mặt vật liệu kim loại có độ nhám ($r$), hiện tượng tán xạ vi mô được mô phỏng bằng cách cộng thêm một vector dịch chuyển ngẫu nhiên phân bố trong hình cầu đơn vị, nhân với hệ số độ nhám $r$, vào hướng phản xạ lý tưởng:
$ R' = "normalize"(R + r dot v_"random") $

- Độ nhám $r$ càng lớn thì chùm tia phản xạ càng tản rộng, tạo hiệu ứng bề mặt mờ (fuzziness).
- Nếu vector hướng mới thỏa điều kiện $R' dot N <= 0$ (hướng phản xạ bị lệch và chui xuống dưới bề mặt), hướng này sẽ được phản xạ ngược lại qua pháp tuyến $N$ ($R' = R' - 2(R' dot N)N$) để đảm bảo tia sáng tiếp tục lan truyền ở nửa không gian phía trên bề mặt.
===== định luật khúc xạ snell
Khi ánh sáng truyền qua ranh giới giữa hai môi trường có chiết suất khác nhau $n_1$ và $n_2$, định luật Snell chi phối hiện tượng khúc xạ ánh sáng:
$ n_1 sin theta_1 = n_2 sin theta_2 $

Đặt tỉ số chiết suất $eta = n_1 / n_2$ và góc tới $cos theta = -D dot N$ (với $N$ luôn hướng về phía tia tới). Vector hướng tia khúc xạ $R$ được phân rã thành hai thành phần vuông góc và song song với mặt phẳng tiếp tuyến:
$ R_perp = eta (D + cos theta N) $
$ R_parallel = -sqrt(1 - |R_perp|^2) N $
$ R = R_perp + R_parallel $

- *Hiện tượng phản xạ toàn phần (Total Internal Reflection):* Khi ánh sáng truyền từ môi trường chiết suất cao sang môi trường chiết suất thấp ($n_1 > n_2$) dưới góc tới lớn, điều kiện $sin^2 theta_2 = eta^2 (1 - cos^2 theta) > 1$ có thể xảy ra. Khi đó, không tồn tại tia khúc xạ và toàn bộ năng lượng bức xạ bị phản xạ hoàn toàn theo định luật phản xạ thông thường.
- *Hệ số phản xạ Fresnel (Xấp xỉ Schlick):* Ngay cả khi góc tới cho phép khúc xạ xảy ra, một phần năng lượng ánh sáng vẫn bị phản xạ ngược lại bề mặt, và tỉ lệ này tăng lên rõ rệt khi góc tới trở nên xiên góc. Chương trình sử dụng công thức xấp xỉ Schlick để tính hệ số phản xạ theo góc:
$ R_0 = ((1 - eta)/(1 + eta))^2 $
$ R(theta) = R_0 + (1 - R_0)(1 - cos theta)^5 $
- Thay vì tách nhánh tia thành hai phần (làm tăng theo cấp số nhân số lượng tia cần dò), chương trình áp dụng phương pháp lấy mẫu ngẫu nhiên (Monte Carlo sampling): với xác suất $R(theta)$, tia được chọn phản xạ; ngược lại, tia sẽ khúc xạ. Qua nhiều lượt dựng hình tích lũy, giá trị trung bình sẽ hội tụ chính xác về tỉ lệ năng lượng quang học thực tế.
===== cơ chế tán xạ bề mặt nhám
Bề mặt khuếch tán lý tưởng (vật liệu Matte) tuân theo mô hình phản xạ Lambertian: ánh sáng tới bị tán xạ đều đặn theo mọi hướng trong bán cầu phía ngoài, và độ chói quan sát được không phụ thuộc vào góc nhìn. Theo định luật cosin Lambert, mật độ năng lượng tán xạ theo một hướng bất kỳ tỉ lệ thuận với $cos theta$, với $theta$ là góc giữa hướng tán xạ và pháp tuyến mặt phẳng.

Để lấy mẫu hướng tán xạ tuân theo phân bố trọng số cosin này, chương trình thực hiện cộng trực tiếp vector pháp tuyến với một vector đơn vị ngẫu nhiên phân bố đều trên bề mặt cầu đơn vị:
$ D_"new" = "normalize"(N + v_"unit random") $

- Trường hợp tổng $N + v$ xấp xỉ bằng vector không (do vector ngẫu nhiên ngược hướng hoàn toàn với pháp tuyến), thuật toán sẽ gán trực tiếp $D_"new" = N$ để tránh lỗi chia cho số không (division by zero).
- Vì phương pháp lấy mẫu này tự động tạo ra phân bố xác suất tỉ lệ với $cos theta$, hệ số $cos theta$ trong phương trình tích phân ánh sáng sẽ triệt tiêu hoàn toàn với hàm mật độ xác suất (PDF); đây chính là một dạng lấy mẫu quan trọng (importance sampling) theo phân bố cosin. Do đó, tại mỗi bước tán xạ, thuật toán chỉ cần nhân hệ số suy hao (throughput) của tia với màu sắc phản xạ của bề mặt (albedo).
==== phương pháp monte carlo
*1. Cơ sở lý thuyết Monte Carlo trong phương trình dựng ảnh*
Ánh sáng rời khỏi một điểm bề mặt $x$ theo hướng quan sát $omega_o$ được tổng quát hóa thông qua phương trình tích phân dựng ảnh của Kajiya (Rendering Equation):
$ L_o(x, omega_o) = L_e(x, omega_o) + integral_Omega f_r(x, omega_i, omega_o) dot L_i(x, omega_i) dot cos theta_i dif omega_i $

Vì tích phân này trải rộng trên toàn bộ bán cầu hướng tới $Omega$ và mang tính đệ quy phức tạp (thành phần ánh sáng tới $L_i(x, omega_i)$ đồng thời là ánh sáng rời từ một điểm va chạm khác trong không gian), phương trình này hoàn toàn không có nghiệm giải tích đối với các cảnh 3D tổng quát.

Để giải quyết bài toán, thuật toán Monte Carlo được sử dụng để xấp xỉ giá trị tích phân thông qua trung bình cộng của các mẫu ngẫu nhiên:
$ integral f(x) dif x approx 1/N sum_(i=1)^N (f(x_i))/(p(x_i)) $

trong đó $x_i$ là các biến ngẫu nhiên được lấy mẫu theo hàm mật độ xác suất $p(x_i)$.

Theo Luật số lớn (Law of Large Numbers), ước lượng này sẽ hội tụ về giá trị đúng khi số lượng mẫu $N$ tiến tới vô cùng. Tuy nhiên, sai số chuẩn (standard error) của phương pháp giảm theo tỉ lệ $cal(O)(1/sqrt(N))$, đồng nghĩa với việc để giảm một nửa mức độ nhiễu hạt, hệ thống cần tăng số lượng mẫu lên gấp bốn lần ($4N$).

*2. Hiện thực hóa phương pháp Monte Carlo trong chương trình*

Trong cấu trúc của hệ thống Ray Tracer, phương pháp Monte Carlo được tích hợp và ứng dụng triệt để tại nhiều thành phần cốt lõi:
- *Dựng hình lũy tiến (Progressive Accumulation)*: Mỗi lượt dựng hình (render pass) độc lập đóng vai trò là một mẫu Monte Carlo cho từng điểm ảnh. Màu sắc hiển thị trên màn hình là giá trị trung bình cộng tích lũy của toàn bộ các lượt qua thời gian. Nhờ đó, hình ảnh xuất hiện với hiện tượng nhiễu nặng ở các lượt đầu tiên và mượt mà, sắc nét dần khi số lượng mẫu tăng lên.
- *Lấy mẫu hướng tán xạ bề mặt (Scattering Direction Sampling)*: Tại các giao điểm có thuộc tính vật liệu khuếch tán nhám (Matte) hoặc kim loại có độ nhám (Metal), hướng của tia phản xạ/tán xạ mới không cố định mà được chọn ngẫu nhiên theo phân bố xác suất tương ứng với hàm phân bố phản xạ hai hướng (BRDF), giúp mô phỏng chính xác hiện tượng bóng đổ mềm và độ mờ bề mặt.
- *Lựa chọn phân nhánh xác suất Schlick (Dielectric Branching)*: Đối với vật liệu trong suốt (Dielectric), hệ thống không tách nhánh tia thành hai tia phản xạ và khúc xạ riêng biệt, mà sử dụng xác suất phản xạ Fresnel Schlick $R(theta)$ làm ngưỡng ngẫu nhiên. Qua nhiều lượt lấy mẫu, tỉ lệ hội tụ tự động cân bằng năng lượng quang học chính xác giữa hai hiện tượng.- *Khử răng cưa (Anti-aliasing) và Độ sâu trường ảnh (Depth of Field)*:
    - Khử răng cưa được thực hiện bằng cách cộng một độ lệch ngẫu nhiên nhỏ (trong hình tròn, độ lớn điều chỉnh bởi tham số khử răng cưa của camera) vào hướng tia sơ cấp. Mỗi lượt chỉ dò một tia cho mỗi điểm ảnh, các lượt sau bổ sung mẫu mới.
    - Hiệu ứng độ sâu trường ảnh (depth of field) được mô phỏng bằng cách dịch chuyển ngẫu nhiên vị trí gốc tia xuất phát trên một mặt phẳng thấu kính (khẩu độ tròn - circular aperture), làm ảnh bị nhòe với mức độ phụ thuộc vào khoảng cách tới camera. Cách này chỉ dịch gốc tia mà giữ nguyên hướng tia nên chỉ là một xấp xỉ của độ sâu trường ảnh, chưa có mặt phẳng lấy nét thực sự.
- *Kỹ thuật dừng tia ngẫu nhiên (Russian Roulette)*: Để ngăn chặn hiện tượng đệ quy quá sâu hoặc vô tận làm suy giảm hiệu năng, chương trình áp dụng kỹ thuật Russian Roulette từ lần nảy thứ 5 trở đi. Tia sáng sẽ bị loại bỏ với xác suất $1 - p$, trong đó $p$ được xác định bằng thành phần cường độ màu lớn nhất của hệ số suy hao (throughput). Các tia sống sót sẽ được hiệu chỉnh trọng số bằng cách nhân ngược với hệ số $1/p$.

== tổ chức cấu trúc dữ liệu và thuật toán
=== phát biểu bài toán
==== Về tính toán
- Cài đặt hệ thống hình học toán học cơ bản (Vector, Tia, Hộp bao AABB, Hình cầu, Tam giác).
- Xử lý tính toán giao cắt tia với các hình học cơ bản (mặt cầu) và phức tạp (lưới đa giác tạo từ nhiều tam giác).
- Mô phỏng các tính chất vật liệu quang học: phản xạ gương (kim loại), khúc xạ (thủy tinh/nước), tán xạ khuếch tán, suy hao và tán xạ trong môi trường thuần nhất (nhựa, khói).
- Tính toán ánh sáng toàn cục bằng thuật toán Path Tracing dựa trên mô phỏng Monte Carlo.
- Ứng dụng cơ chế lấy mẫu quan trọng trong lý thuyết xác suất thống kê để hội tụ hình ảnh kết xuất nhanh hơn.

==== Về cấu trúc dữ liệu
- Thiết kế mô hình các đối tượng linh hoạt, có tính đóng gói cao và dễ dàng mở rộng/tái sử dụng.
- Cài đặt cấu trúc dữ liệu cây phân vùng không gian để tăng tốc độ truy vấn giao điểm giữa tia và hàng ngàn vật thể trong không gian.
- Ứng dụng đa luồng để phân chia công việc tính toán các điểm ảnh lên nhiều lõi CPU.

==== Về giao diện
- Xây dựng không gian làm việc trực quan bằng SFML và ImGui.
- Xây dựng các chức năng tương tác thời gian thực:
  - Lưu và tải cảnh.
  - Thay đổi thuộc tính vật thể (vị trí, góc quay, tỉ lệ, bán kính, màu sắc, chiết suất, độ nhám, cường độ sáng).
  - Điều hướng Camera (vị trí, điểm nhìn, tiêu cự, khẩu độ) và cửa sổ Postprocessing cho độ phơi sáng, gamma và toán tử tone mapping.
  - Chọn vật thể trực tiếp trên khung hình.
  - Thay đổi độ phân giải khung hình, số lần dội tối đa và số mẫu mỗi điểm ảnh.
  - Tạm dừng/tiếp tục và đặt lại quá trình tích lũy.

=== cấu trúc dữ liệu

==== cây phân vùng
Thay vì duyệt tuyến tính qua toàn bộ danh sách vật thể để tìm giao điểm, chương trình sử dụng cấu trúc cây BVH để truy vấn giao cắt giữa tia và các vật thể trong cảnh. Cấu trúc của một nút trên cây gồm con trỏ đến 2 nút con (trỏ đến vô định nếu là nút lá), phạm vi vật thể mà nó quản lý và hộp bao AABB của những vật thể đó. Hai nút con của một nút cha sẽ quản lý 2 tập con bù nhau của tập vật thể mà nút cha quản lý.

Khi kiểm tra giao cắt, ta kiểm tra xem tia có cắt với hộp bao của nút BVH không. Nếu có, ta phải lặp lại phép kiểm tra tương tự trên cả 2 nút con của nút còn lại (nếu là nút lá thì tiến hành kiểm tra giao cắt giữa tia - vật thể); nếu không, ta có thể hoàn toàn bỏ qua việc kiểm tra các nút con của nút hiện tại, do đó giảm đáng kể thời gian truy vấn.

Sử dụng cây BVH, ta có thể giảm độ phức tạp thời gian truy vấn từ $O(N)$ đối với tìm kiếm tuần tự xuống trung bình $O(log N)$, với $N$ là số lượng vật thể trong cảnh.

==== mảng động

==== ngăn xếp

=== thuật toán xử lý
Trong các thuật toán, $epsilon$ là mức sai số chấp nhận được dùng trong các phép so sánh số thực.

==== Thuật toán giao cắt tia – mặt cầu

===== Đầu vào
- Tia cần kiểm tra có gốc $arrow(o)$ và hướng $arrow(d)$.
- Mặt cầu có bán kính $r$. Giả định tâm hình cầu luôn đặt tại `(0, 0, 0)`.
- Vật liệu của mặt cầu.

===== Đầu ra
- Một cấu trúc `HitInfo` chứa khoảng cách, điểm giao, pháp tuyến, cờ `front_face` và vật liệu của mặt cầu nếu tồn tại giao điểm.

===== Phương pháp
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

Để tránh chuyển vị $R^T$ và chia cho $S$ trên mỗi tia, `Transform` lưu sẵn `inverse_rotation` $= R^T$ và `inverse_scale` $= 1/S$. Hai trường này được cập nhật trong `rotate()`, `size()` và `refreshInverse()` mỗi khi phép biến đổi thay đổi. Nhờ vậy `pointApplyInverse`/`dirApplyInverse` chỉ còn một phép nhân ma trận - vector.

Phép biến đổi nghịch đảo đưa tia vào không gian cục bộ, nơi mặt cầu vẫn là mặt cầu bán kính $r$. Sau đó chạy thuật toán giao cắt với mặt cầu bán kính $r$ đặt tại gốc tọa độ như bình thường để có được giao điểm $arrow(d)_"local"$ và pháp tuyến $arrow(n)_"local"$ trên không gian cục bộ.

Sau đó các đại lượng được đưa trở lại không gian thế giới:

$ arrow(p)_"world" = arrow(T) + R S arrow(p)_"local", quad arrow(n)_"world" = "normalize"(R S^(-1) arrow(n)_"local") $

Khoảng cách được tính nhanh bằng cách lấy hình chiếu của vector khoảng cách lên hướng tia thế giới:

$ t_"world" = (arrow(p)_"world" - arrow(o)_"world") dot arrow(d)_"world" $

Do $arrow(p)_"word"$, $arrow(d)_"world"$ cùng hướng và $|arrow(d)_"world"| = 1$, $t_"world"$ chính là khoảng cách thực dọc theo tia. Cuối cùng, các giá trị này được đóng gói vào `HitInfo` và trả về.

Độ phức tạp của phép kiểm tra là $O(1)$.

==== Thuật toán giao cắt tia – hộp bao AABB

===== Đầu vào
- Tia cần kiểm tra `ray` với gốc `origin` và hướng `direction`.
- `inv_direction` $= 1/"direction"$, được tính một lần trước khi duyệt BVH.
- Hộp bao `AABB` được biểu diễn bởi hai điểm đối diện `minp` và `maxp`.
- Khoảng tham số hợp lệ $[t_min, t_max]$. Trong quá trình duyệt BVH, $t_min$ thường bằng `EPSILON` và $t_max$ bằng khoảng cách tới va chạm gần nhất hoặc `FAR_DISTANCE`.

===== Đầu ra
- `true` nếu tia cắt hộp bao trong khoảng $[t_min, t_max]$.
- `false` nếu tia không cắt hộp bao, cắt ở phía sau gốc tia, hoặc chỉ cắt ngoài khoảng tham số cho phép.

===== Phương pháp
AABB là giao của ba "slab" song song với các mặt phẳng tọa độ $x$, $y$, $z$. Với mỗi trục $a$, giao điểm của tia với hai mặt phẳng $a = "minp"_a$ và $a = "maxp"_a$ là:

$ t_0 = ("minp"_a - o_a)/d_a, quad t_1 = ("maxp"_a - o_a)/d_a $

Nếu $d_a < 0$, thứ tự hai nghiệm bị đảo nên chương trình hoán đổi $t_0$ và $t_1$. Khoảng giao của tia với slab thứ $a$ là $[t_0, t_1]$. Khoảng giao tổng của tia với cả ba slab là giao của ba khoảng:

$ t_"enter" = max(t_"enter", t_0), quad t_"exit" = min(t_"exit", t_1) $

Ban đầu $t_"enter" = t_min$ và $t_"exit" = t_max$. Sau mỗi trục, nếu $t_"exit" < t_"enter"$ thì khoảng giao rỗng và tia không cắt hộp bao. Nếu duyệt hết cả ba trục mà điều kiện vẫn đúng, tia cắt hộp bao.

Về mặt hình học, $t_"enter"$ là tham số mà tia bắt đầu đi vào hộp, còn $t_"exit"$ là tham số mà tia đi ra khỏi hộp. Nếu $t_"exit" < t_"enter"$, tia đi qua các slab theo thứ tự không hợp lệ, tức là không có đoạn nào nằm đồng thời trong cả ba slab.

```cpp
// inv_direction = 1/ray.direction, được tính một lần cho cả lượt duyệt BVH
for (int a = 0; a < 3; ++a) {
    float invD = inv_direction.axis[a];
    float t0 = (minp.axis[a] - ray.origin.axis[a]) * invD;
    float t1 = (maxp.axis[a] - ray.origin.axis[a]) * invD;

    if (invD < 0.0f) std::swap(t0, t1);

    tmin = t0 > tmin ? t0 : tmin;
    tmax = t1 < tmax ? t1 : tmax;

    if (tmax < tmin) return false;
}
return true;
```

Nghịch đảo hướng tia `1/direction` được tính một lần ở đầu `BVHNode::traverse` rồi truyền vào `AABB::hit`, thay vì chia lại cho từng nút. Mỗi tia thường đi qua hàng chục nút nên việc này loại bỏ phần lớn phép chia.

Trường hợp $d_a = 0$ được xử lý dựa trên số học dấu phẩy động của IEEE 754: `invD` là $plus.minus infinity$, nên $t_0$ và $t_1$ trở thành các giá trị vô cực. Cách này cho kết quả đúng khi gốc tia không nằm đúng trên mặt slab. Nếu gốc tia nằm chính xác trên mặt slab và $d_a = 0$, phép nhân $0 dot infinity$ có thể tạo ra `NaN`; khi đó cần xử lý riêng nếu muốn đảm bảo ổn định số học tuyệt đối.

Phép kiểm tra AABB là cơ sở để cắt tỉa cây BVH. Trong `BVHNode::traverse`, mỗi nút được kiểm tra bằng `current_node->aabb.hit(ray, inv_direction, EPSILON, t_max)`, với `inv_direction = 1/ray.direction` được tính một lần trước khi duyệt. Nếu kết quả là `false`, toàn bộ nhánh con của nút bị bỏ qua; nếu là `true`, thuật toán tiếp tục đi xuống nút lá để kiểm tra chi tiết với các hình học bên trong.

Độ phức tạp của một phép kiểm tra AABB là $O(1)$ vì chỉ cần xử lý ba trục.

==== Thuật toán giao cắt tia - tam giác (Moller - Trumbore)

===== Đầu vào
- Tia cần kiểm tra với gốc $arrow(o)$ và hướng $arrow(d)$.
- Tam giác với ba đỉnh $arrow(v)_0, arrow(v)_1, arrow(v)_2$ và ba tọa độ kết cấu tương ứng $arrow("uv")_0, arrow("uv")_1, arrow("uv")_2$.
- Khoảng tham số hợp lệ $[t_min, t_max]$.

===== Đầu ra
- `true` nếu tia cắt tam giác trong khoảng tham số cho phép.
- Khoảng cách $t$ và hai tọa độ barycentric $u$, $v$ của giao điểm nếu tồn tại.

===== Phương pháp
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

==== xây dựng BVH
===== Đầu vào
- Danh sách các vật thể trong khoảng `[start, end)`, với `start` và `end` là chỉ số phần tử.
- Ngưỡng số vật thể tối đa trong một nút lá `PRIM_PER_LEAF`.

===== Đầu ra
- Nút gốc của cây BVH đối với các vật thể từ `start` đến `end` trong danh sách đã cho.

===== Phương pháp
+ Nếu `start >= end`, trả về một nút rỗng (`num_objects = 0`) để tránh truy cập ngoài danh sách.
+ Tính AABB bao trọn toàn bộ vật thể từ `start` đến `end`.
+ Nếu số lượng vật thể `size = end - start` nhỏ hơn hoặc bằng `PRIM_PER_LEAF`, nút hiện tại là nút lá: `left` và `right` bằng `nullptr`, lưu `start_index = start` và `num_objects = size`, rồi kết thúc đệ quy.
+ Ngược lại, chọn trục có chiều dài lớn nhất `axis` của AABB (trục $x$, $y$ hoặc $z$).
+ Sắp xếp các vật thể tăng dần theo tọa độ nhỏ nhất (`minp[axis]`) của AABB tương ứng trên trục vừa chọn.
+ Chia danh sách đã sắp xếp thành hai nửa bằng nhau và gọi đệ quy để tạo `left`, `right`.
+ AABB của nút hiện tại là `span_box` đã tính ở trên; giá trị này tương đương với hợp AABB của `left` và `right`.

Để tạo cây BVH của toàn bộ vật thể trong danh sách `objects`, chỉ cần gọi `BVHNode(objects, 0, objects.size())`.

==== duyệt BVH
===== Đầu vào
- Nút gốc `bvh_root` của cây BVH.
- Tia sáng `ray`.
- Hàm callback `hitFunc(primitive_idx, closest_hit)` để kiểm tra chi tiết từng vật thể trong nút lá.

===== Đầu ra
- `HitInfo` chứa thông tin về điểm giao gần nhất (nếu có); `distance = -1` nếu không cắt.

===== Phương pháp
Chương trình duyệt lặp bằng hàng đợi `std::queue<const BVHNode*>`, không đệ quy:

+ Khởi tạo `node_queue` chứa nút gốc `bvh_root`, `closest_hit` rỗng.
+ Lặp cho đến khi hàng đợi rỗng:
  + Lấy `current_node = node_queue.front()` rồi gọi `node_queue.pop()`.
  + Lấy `t_max = closest_hit.distance`; nếu `closest_hit` rỗng thì `t_max = FAR_DISTANCE` (tức vô cùng).
  + Kiểm tra `current_node->aabb.hit(ray, inv_direction, EPSILON, t_max)`, với `inv_direction` đã tính sẵn:
    - Nếu không cắt: bỏ qua lượt này.
    - Nếu cắt và là nút nhánh (`left`/`right` khác `nullptr`): thêm `left` và `right` vào hàng đợi.
    - Nếu cắt và là nút lá: gọi `hitFunc` cho từng vật thể trong nút; `closest_hit` được cập nhật sau mỗi va chạm.
+ Trả về `closest_hit`.

Nhờ `t_max` được thu hẹp sau mỗi va chạm, các nút có AABB nằm xa hơn điểm giao gần nhất bị loại bỏ sớm. Độ phức tạp trung bình là $O(log N)$ cho mỗi tia.

==== Thuật toán tính hướng khúc xạ
===== Đầu vào
- Hướng tia tới $arrow(d)$
- Pháp tuyến trên vật liệu tại tiếp điểm $arrow(n)$
- Mặt tiếp xúc vật thể (mặt trong hoặc ngoài)
- Chiết xuất của môi trường chứa tia $n_1$
- Chiết xuất của môi trường tia hướng đến $n_2$

===== Đầu ra
- Hướng tia mới $arrow(d)_"new"$
- Kết quả: bị tán xạ `Scattered` hoặc bị khúc xạ `Refracted`.

===== Phương pháp
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

==== Thuật toán tính hướng tán xạ
===== Đầu vào
- Hướng tia ban đầu $arrow(d)$.
- Độ nhám vật liệu $r$ có giá trị từ $0$ đến $1$.

===== Đầu ra
- Hướng tia mới $arrow(d)_"new"$.
- Kết quả: luôn bị tán xạ `Scattered`.

===== Phương pháp
Với vật liệu kim loại `Metal`, hướng phản xạ gương được tính bởi:
$ arrow(d)_"spec" = arrow(d) - 2(arrow(d) dot arrow(n))arrow(n) $

Nếu độ nhám $r = 0$, tia phản xạ gương được dùng trực tiếp và không lấy mẫu ngẫu nhiên. Nếu $r > 0$, chương trình lấy một mẫu $arrow(v)_"cos"$ theo phân phối cosine quanh $arrow(d)_"spec"$ rồi nội suy tuyến tính theo $r$:
$ arrow(d)_"new" = "normalize"((1 - r) arrow(d)_"spec" + r arrow(v)_"cos") $

Do $arrow(v)_"cos"$ luôn nằm trong bán cầu quanh $arrow(d)_"spec"$, công thức này tạo ra một thùy sáng có bề rộng tăng dần theo $r$. Khi hướng phản xạ gương gần sát đường chân trời, thùy vẫn có thể lấn xuống dưới mặt bề mặt; khi đó vector được phản xạ lại qua $arrow(n)$ trước khi chuẩn hóa. Phép kiểm tra này được viết dưới dạng chọn (select) thay vì rẽ nhánh, vì ở độ nhám lớn nó xảy ra thường xuyên và bộ dự đoán nhánh sẽ đoán sai nhiều.

Vật liệu nhám `Matte` tương ứng với trường hợp $r = 1$ của công thức trên khi tia tới trùng với pháp tuyến ($arrow(d)_"spec" = arrow(n)$), nên thùy sáng thu về đúng phân phối cosine quanh pháp tuyến:
$ arrow(d)_"new" = arrow(v)_"cos" $

trong đó $arrow(v)_"cos"$ là một vector đơn vị ngẫu nhiên theo phân phối cosine quanh $arrow(n)$.

==== thuật toán dò tia

===== Đầu vào
- Tia sơ cấp `ray`.
- Cấu trúc dữ liệu chứa các vật thể cần kết xuất (cây BVH), cùng môi trường toàn cục `global_medium`.
- Số lần dội tối đa `max_bounces`.

===== Đầu ra
- Màu của tia sau khi dò.

===== Phương pháp
+ *Khởi tạo:* đặt `final_color = (0, 0, 0)`, `throughput = (1, 1, 1)` và một ngăn xếp môi trường `medium_stack` (mảng tĩnh, kích thước tối đa 64) chứa môi trường toàn cục `global_medium`.
+ *Dò tia:* lặp tối đa `max_bounces` lần. Ở mỗi lần dội, thực hiện lần lượt các bước sau:
  + *Kiểm tra giao cắt:* tìm giao điểm gần nhất của `ray` với các vật thể, lưu vào `rec` và `obj`. Nếu tia không cắt vật thể nào thì `obj = nullptr` và `rec.distance = -1`; trường hợp này được xử lý sau bước tương tác môi trường.

  + *Tương tác với môi trường:* với `current_medium` là đỉnh của `medium_stack`, đặt `travel_distance = rec.distance` nếu có vật thể, ngược lại `travel_distance = FAR_DISTANCE` (tia đi về phía bầu trời, tránh dùng giá trị `-1` của `HitInfo`). Gọi `transmitted_distance = current_medium->transmit(ray, throughput, travel_distance)`.
    - Nếu `transmitted_distance < travel_distance`: tia bị tán xạ thể tích trước khi tới bề mặt (hoặc trước khi tới bầu trời). `transmit()` đã dời `ray.origin` tới điểm tán xạ và chọn hướng mới; quay lại đầu vòng lặp để dò tia mới.
    - Ngược lại: tia tới được bề mặt; `throughput` đã được nhân với suy hao của đoạn đường.
    - Với `Homogeneous`: nếu `sigma_s = 0`, `transmit()` dùng Beer–Lambert tất định với `sigma_a` và trả về `distance`; nếu `sigma_s > 0`, hàm lấy mẫu quãng đường bay tự do với tốc độ `majorant` và trả về quãng đường thực tế. Trọng số tán xạ `eff_sigma_s / majorant` và phần dư suy hao `exp(-(eff_sigma_t - majorant) * t)` được nhân vào `throughput`; nếu không có va chạm, tia tiếp tục tới bề mặt.

  + *Không có vật thể:* nếu `obj == nullptr` sau bước tương tác môi trường, tính toán ánh sáng nền (phần SkyBox hiện chưa cài đặt) và thoát khỏi vòng lặp.

  + *Phát xạ:* `final_color += throughput * material->emitted(rec)`. `Material::emitted` mặc định trả về màu đen; `Light` trả về `texture->get(rec) * emission_strength`.

  + *Tính toán tia mới:* khởi tạo `surface_attenuation = (1, 1, 1)` rồi gọi `material->scatter(ray, surface_attenuation, rec, current_medium)`; hàm này vừa tính toán suy hao bề mặt, vừa tính toán hướng tia mới (dựa trên thuật toán tán xạ/khúc xạ) và trả về kết quả tán xạ/khúc xạ/kết thúc dò tia. Nếu như kết quả là
    - `Terminated`: tia gặp nguồn sáng, thoát khỏi vòng lặp.
    - `Scattered`: tia bị tán xạ. Nhân `throughput *= surface_attenuation` rồi tiếp tục với tia mới
    - `Refracted`: tia bị khúc xạ. `throughput` không đổi (suy hao dọc đường là việc của môi trường); cập nhật `medium_stack`: nếu cắt mặt ngoài của vật liệu thì đẩy môi trường của vật liệu vào; nếu không và đỉnh ngăn xếp đúng là môi trường đó thì lấy ra.
  + *Russian roulette:* ở đầu mỗi lần dội, khi chỉ số dội `bounces` đủ lớn, đặt `m = max(throughput.x, throughput.y, throughput.z)`. Nếu `RNG::uniform() > m` thì kết thúc tia; ngược lại chia `throughput` cho `m` để bảo toàn năng lượng kỳ vọng. Điều này giúp thiên vị các tia mang ánh sáng mạnh, kết thúc sớm các tia mang ánh sáng yếu nhằm giảm thời gian dò tia cho các tia đem lại hiệu quả thấp.
  + *Lặp lại:* với tia `ray` vừa cập nhật, quá trình tiếp tục cho đến khi tia kết thúc hoặc đạt giới hạn số lần dội.
+ Sau khi thoát khỏi vòng lặp, trả về `final_color` là màu đã dò được từ tia sơ cấp ban đầu.

==== Thuật toán kết xuất bằng phương pháp Monte Carlo

===== Đầu vào
- `Camera` gồm vị trí `position`, hệ trục ảnh `(w, u, v)` là các cơ sở ở không gian cục bộ của camera, tiêu cự `focal_length` và khẩu độ `aperture`.
- `Scene` chứa cây BVH, vật thể, vật liệu và môi trường toàn cục.
- Kích thước ảnh `W × H`, tương ứng với `viewport_width` và `viewport_height`.
- Số mẫu mỗi pixel `samples_per_pixel` (gọi tắt là `spp`) và số lần dội tối đa `max_bounces`.
- Bộ đệm tích lũy `buffer` và số lượt đã kết xuất `pass`.

====== Đầu ra
- `buffer` HDR được cộng dồn để các lượt sau tiếp tục hội tụ; `pass` tăng thêm 1 sau mỗi lần kết xuất.
- `RayTracer::render()` không hậu kỳ và không tạo ảnh 8-bit; bước đó chỉ chạy khi hiển thị hoặc lưu ảnh.

====== Phương pháp
Mỗi pixel nhận ánh sáng từ một tập tia liên tục trên diện tích pixel và trên mặt phẳng khẩu độ. Gọi $I_(i j)$ là màu lý tưởng của pixel ở cột $i$, hàng $j$. Giá trị này là một tích phân trên miền mẫu, nhưng chương trình ước lượng bằng Monte Carlo: lấy $N$ mẫu ngẫu nhiên, tạo một tia cho mỗi mẫu, dò màu rồi lấy trung bình:

$ I_(i j) approx 1/N sum_(k=1)^N L(arrow(r)_k) $

trong đó $L(arrow(r)_k)$ là màu trả về bởi `trace()` cho tia thứ $k$. Vì bản thân `trace()` cũng dùng mẫu ngẫu nhiên cho tán xạ, khúc xạ và Russian roulette, toàn bộ ảnh là một ước lượng Monte Carlo nhiều tầng: lấy mẫu pixel, lấy mẫu khẩu độ và lấy mẫu đường đi ánh sáng. Khi $N$ tăng, phương sai giảm theo $O(1/N)$, nên ảnh hội tụ dần.

====== Lấy mẫu pixel và khử răng cưa
- Xét pixel ở cột `i`, hàng `j`. Chia pixel thành lưới $n times n$, với $n = max(1, floor(sqrt("spp")))$. Số mẫu thực tế mỗi pixel mỗi lượt là $N = n^2$; nếu `spp` không phải số chính phương thì $N$ nhỏ hơn giá trị đã đặt.
- Với mỗi ô $(s_x, s_y)$ trong lưới, lấy một điểm ngẫu nhiên bên trong ô:

$ x = (i + (s_x + xi_x)/n)/W - 1/2, quad y = -((j + (s_y + xi_y)/n)/H - 1/2) $

trong đó $xi_x, xi_y$ là hai biến ngẫu nhiên phân phối đều trong $[0, 1)$. Dấu trừ ở $y$ đảo trục tung vì tọa độ ảnh tăng từ trên xuống, còn trục $v$ của camera hướng lên.
- Đặt $"uv" = (x, y)$ và nhân từng thành phần tương ứng với $"world_viewport" = (2 W/H, 2)$ để được tọa độ viewport $"uv_viewport" = (x_v, y_v)$, với $x_v = 2 x W / H$ và $y_v = 2 y$. Sau đó tạo tia bằng `camera.getRayAt(uv_viewport)`. Hướng tia trước khi chuẩn hóa là:

$ arrow(d) = f arrow(w) + x_v arrow(u) + y_v arrow(v) $

trong đó $f$ là `focal_length` và $(arrow(w), arrow(u), arrow(v))$ là các cơ sở của `camera`.
- Vì mỗi mẫu nằm ở một vị trí khác nhau trong pixel, biên vật thể được lấy trung bình theo độ phủ thực để khử răng cưa (anti-aliasing). Cách chia lưới rồi lấy ngẫu nhiên trong từng ô là lấy mẫu phân tầng (stratified sampling), giúp giảm phương sai so với chọn hoàn toàn ngẫu nhiên.

====== Khẩu độ và độ sâu trường ảnh
Khi gọi `Camera::getRayAt()` với `aperture > 0` và `jitter = true`, hàm dịch gốc tia trong mặt phẳng khẩu độ sinh bởi hai vector cơ sở $arrow(u)$ và $arrow(v)$ bằng một offset ngẫu nhiên. Lấy mẫu ngẫu nhiên $(Delta u, Delta v)$ bằng `RNG::pointInCircle(aperture/2)`; gọi $arrow(o)$ là gốc tia trước khi dịch; gốc mới $arrow(o)'$ là:

$ arrow(o)' = arrow(o) + Delta u arrow(u) + Delta v arrow(v) $

Việc lấy nhiều gốc tia khác nhau trong cùng một pixel làm ảnh bị nhòe với các mức khác nhau tùy vào khoảng cách của các vật so với vị trí của `camera`; đây là cơ chế được dùng để xấp xỉ độ sâu trường ảnh (depth of field). Khi `aperture = 0`, gốc tia không dịch và ảnh chỉ còn khử răng cưa.

====== Tích lũy qua nhiều lượt
Gọi `accumulate` là tổng màu của $N$ mẫu trong một pixel ở lượt tích lũy hiện tại; `accumulate / N` là ước lượng của lượt đó. Giá trị này được cộng vào `buffer`.

Gọi $I_p$ là ước lượng màu của lượt tích lũy thứ $p$ và $P$ là giá trị hiện tại của `pass`. Màu trung bình của pixel sau $P + 1$ lượt là:

$ I_P = 1/(P + 1) sum_(p=0)^P I_p $

Vì mỗi lượt dùng một tập mẫu ngẫu nhiên mới từ cùng một hàm phân phối, ảnh dần hội tụ theo định lý giới hạn trung tâm.

====== Hậu kỳ
Hậu kỳ không nằm trong `RayTracer::render()`; nó được tách thành hàm `resolvePixels()` và chỉ chạy trên luồng giao diện khi hiển thị hoặc khi lưu ảnh. Nhờ vậy thời gian kết xuất chỉ gồm việc dò tia và cộng dồn `buffer`.

Gọi `display` là màu trung bình của pixel sau khi chia `buffer` cho số lượt: `display = buffer / pass`. Giá trị này vẫn là giá trị màu tuyến tính và có thể lớn hơn 1. Chương trình áp dụng các bước sau theo đúng thứ tự:

- *Tone mapping:* nén dải động (HDR) về không gian hiển thị chuẩn hóa $[0, 1]$. Có hai toán tử chọn được:
  - *Exposure:* $T(c, e) = min(1 - exp(-e c), 1)$. Giá trị âm hoặc `NaN` được đưa về 0 trước khi tính. Hàm này giữ được chi tiết ở vùng sáng thay vì cắt trắng đột ngột.
  - *Reinhard:* nhân màu với độ phơi sáng rồi nén bằng $T(c, e) = (e c) / (1 + e c)$. Các kênh âm được kẹp về 0 trước phép chia.

trong đó $c$ là một kênh màu và $e$ là `exposure`.
- *Gamma correction:* mã hóa màu đã tone map sang không gian hiển thị:

$ C_"out" = T(c, e)^(1/gamma) $

`gamma` mặc định là 2.2 và có thể chỉnh trong cửa sổ Postprocessing. Bước này bù cho việc màn hình hiển thị màu theo đường cong gamma xấp xỉ 2.2. Tone mapping phải được thực hiện trước gamma vì tone mapping là phép phi tuyến trên giá trị màu tuyến tính.
- *Lượng tử hóa:* mỗi kênh được kẹp vào $[0, 1]$, nhân với 255, cộng 0.5 rồi lấy phần nguyên để chuyển thành số nguyên 8-bit. `toABGR()` đóng gói ba kênh màu và alpha thành một số nguyên 32-bit (kênh alpha được thêm vào để tương thích với các thư viện đồ họa và được gán bằng 255).

====== Mã giả
```cpp
int n = max(1, (int)sqrt(spp));
int N = n * n;
Vec2 world_viewport = Vec2(2.0f * W / H, 2.0f);

for (each pass) {
    for (each pixel (i, j)) {
        Color accumulate = Color(0);

        for (each stratified sample) {
            Vec2 uv = ...;  // pixel coordinates in [-0.5, 0.5]
            Vec2 uv_viewport = uv * world_viewport;
            Ray ray = camera.getRayAt(uv_viewport);
            accumulate += trace(ray, scene);
        }

        buffer[i, j] += accumulate / N;
    }

    pass++;
}

// chạy trên luồng giao diện, khi hiển thị hoặc lưu ảnh
void resolvePixels(buffer, pixels, pass, exposure, gamma, tonemap) {
    for (each pixel (i, j)) {
        Color display = buffer[i, j] / pass;

        if (tonemap == Tonemap::Reinhard) {
            display = clampNegative(display * exposure);
            reihardTonemap(display);
        } else {
            exposureTonemap(display, exposure);
        }

        display = gammaCorrect(display, gamma);
        pixels[i, j] = display.toABGR();
    }
}
```

Độ phức tạp của một lượt kết xuất là $O(W H N C)$, với $C$ là chi phí trung bình của một lần `trace()`. Chi phí này phụ thuộc vào `max_bounces`, cấu trúc BVH và số vật thể trong cảnh.

Tóm lại, `RayTracer::render()` kết hợp lấy mẫu phân tầng trên pixel, offset khẩu độ và tích lũy nhiều lượt; hậu kỳ tone mapping/gamma được tách sang `resolvePixels()` ở lúc hiển thị/lưu ảnh để không tính vào thời gian kết xuất. Số mẫu mỗi pixel và số lượt càng lớn thì ảnh càng hội tụ, đổi lại thời gian kết xuất tăng tuyến tính.

== cài đặt và triển khai
=== ngôn ngữ cài đặt
Hệ thống được cài đặt hoàn toàn bằng ngôn ngữ C++ theo định hướng hướng đối tượng, kết hợp với các thư viện bên dưới:

- Thư viện đồ họa SFML kết hợp với thư viện giao diện người dùng ImGui.
- Thư viện hộp thoại file nativefiledialog.
- Thư viện đa luồng OpenMP được cài đặt sẵn trong GCC/Clang/MSVC.
- Thư viện chuẩn C++: `std::vector` cho mảng động, `std::random` để sinh số ngẫu nhiên, `std::thread`/`std::atomic`/`std::mutex` để quản lý luồng và đồng bộ hóa.

=== tổ chức chương trình

==== Tổng quan các lớp
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
            "# front_buffer: vector<Color>",
            "# front_pass: unsigned",
            "# display_pixels: vector<uint32_t>",
            "# exposure: float",
            "# gamma: float",
            "# tonemap: Tonemap",
            "# display_dirty: bool",
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
          ),
          (
            "+ RayTracer(width, height)",
            "+ getBuffer(): const vector<Color>&",
            "+ trace(ray: Ray, scene: const Scene&): Color",
            "+ reset()",
            "+ render(camera: const Camera&, scene: const Scene&)",
          )
        ), name: <raytracer>),

        node((-1, 1.3), class(
          "Camera",
          (
            "# focal_length, aperture: float",
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

        edge(<gui>, <raytracer>, marks: composition, bend: 60deg),
        edge(<gui>, <camera>, marks: aggregation),
        edge(<gui>, <scene>, marks: aggregation, bend: -10deg),
        edge(<raytracer>, <camera>, marks: dependency, bend: 10deg),
        edge(<raytracer>, <scene>, marks: dependency),

        edge(<scene>, <bvhnode>, marks: composition, bend: -10deg),
        edge(<bvhnode>, <bvhnode>, marks: composition, bend: 110deg, loop-angle: 150deg),
      )
    ]
  ], caption: [Kiến trúc ứng dụng và cảnh.]
)

`RayTracer` là thư viện độc lập với giao diện: nó chỉ nhận `Camera` và `Scene` khi chạy phương thức `render()`. Điều này giúp cho người sử dụng thư viện có thể dựng/chuyển cảnh, kết xuất ảnh linh hoạt thông qua mã lệnh mà không cần giao diện người dùng.

`Gui` sở hữu một `RayTracer`, nó chạy vòng lặp kết xuất và vòng lặp giao diện trên hai luồng riêng biệt. Vì cả 2 luồng đều có thể truy cập đến dữ liệu cảnh như `scene`, `tracer` và `camera`, `Gui` còn sở hữu `scene_mutex` để khóa cảnh, ngăn việc 2 luồng này cùng truy cập và thay đổi dữ liệu cùng một lúc. Luồng kết xuất chỉ hoạt động khi nó giữ `scene_mutex`, trong khi luồng giao diện luôn hoạt động trong suốt vòng đời chương trình. Mọi thay đổi từ giao diện được đưa vào `pending_actions` và chỉ áp dụng khi luồng giao diện lấy được `scene_mutex`; sau khi áp dụng hết các sửa đổi trong `pending_actions` thì nó ngay lập tức thả `scene_mutex` để luồng kết xuất hoạt động trở lại. Luồng giao diện có thể yêu cầu luồng kết xuất thả `scene_mutex` bằng cờ `lock_requested`. Sau mỗi lượt, luồng kết xuất sao chép bộ đệm HDR từ `tracer` sang `front_buffer` (kèm số lượt `front_pass`) rồi bật cờ `is_data_ready`; luồng giao diện chỉ đọc bộ đệm này khi cờ đã bật, sau đó mới hậu kỳ sang `display_pixels` và tải lên `sf::Texture`. Thiết kế này giúp luồng giao diện luôn luôn hoạt động mà không bị dừng lại, làm cho trải nghiệm người dùng mượt mà.

`BVHNode` cài đặt cấu trúc cây phân vùng không gian để tăng tốc tốc độ duyệt giao cắt giữa tia và các vật thể (sẽ được nói rõ hơn ở phần sau).

#figure(
  block(breakable: false, width: 100%)[
    #align(center)[
      #diagram(
        node-shape: rect,
        spacing: (0.2cm, 0.2cm),

        node((0, 0.5), class(
          "NamedElement",
          (
            "# name: string",
          ),
          (
            "+ getName(): string",
          )
        ), name: <namedelement>),

        node((4, 0), class(
          "Hittable",
          ("...",),
          ("...",)
        ), name: <hittable>),

        node((4, 1), class(
          "Material",
          ("...",),
          ("...",)
        ), name: <material>),

        node((-4, 0), class(
          "Texture",
          ("...",),
          ("...",)
        ), name: <texture>),

        node((-4, 1), class(
          "Medium",
          ("...",),
          ("...",)
        ), name: <medium>),

        edge(<hittable>, <namedelement>, marks: inheritance),
        edge(<material>, <namedelement>, marks: inheritance),
        edge(<texture>, <namedelement>, marks: inheritance),
        edge(<medium>, <namedelement>, marks: inheritance),
      )
    ]
  ], caption: [Các đối tượng được đặt tên.]
)

Các lớp `Hittable`, `Material`, `Texture` và `Medium` cùng kế thừa `NamedElement` để dễ dàng quản lý trong giao diện người dùng.

#figure(
  block(breakable: false, width: 100%)[
    #align(center)[
      #diagram(
        node-shape: rect,
        spacing: (0.2cm, 0.2cm),

        node((0.2, 0), class(
          "Transform",
          (
            "# scale: Vec3",
            "# rotation: Mat3x3",
            "# rotation_angles: Vec3",
            "# translation: Vec3",
            "# inverse_rotation: Mat3x3",
            "# inverse_scale: Vec3",
          ),
          (
            "+ refreshInverse()",
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
          ),
          (
            "+ hit(ray: const Ray&): HitInfo = 0",
            "+ getAABB(): AABB",
            "+ getTransform(): Transform&",
            "+ getMaterial(): Material*",
            "- getLocalAABB(): AABB = 0",
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

        node((1.9, 2), class(
          "RawTriangle",
          (
            "# origin, edge1, edge2: Vec3",
            "# uv0, uv1, uv2: Vec2",
          ),
          (
            "+ RawTriangle(v0, v1, v2)",
            "+ RawTriangle(v0, v1, v2, uv0, uv1, uv2)",
          )
        ), name: <rawtriangle>),

        node((1.5, 0), class(
          "BVHNode",
          (
            "...",
          ),
          (
            "...",
          )
        ), name: <bvhnode>),

        edge(<hittable>, <transform>, marks: composition),
        edge(<sphere>, <hittable>, marks: inheritance),
        edge(<mesh>, <hittable>, marks: inheritance),
        edge(<mesh>, <rawtriangle>, marks: composition),
        edge(<mesh>, <bvhnode>, marks: composition, bend: -15deg),
        edge(<bvhnode>, <bvhnode>, marks: composition, bend: -120deg, loop-angle: -150deg),
      )
    ]
  ], caption: [Hệ thống hình học.]
)

Hệ thống hình học được xây dựng quanh lớp cơ sở trừu tượng `Hittable`. Mỗi vật thể lưu một `Transform` (gồm các thông tin về tỉ lệ, góc quay và vị trí) cùng con trỏ tới `Material` là vật liệu của nó. Lớp này khai báo hai phương thức thuần ảo `hit()` và `getLocalAABB()`; các lớp dẫn xuất chỉ cần cài đặt phép giao cắt `hit()` và hàm lấy hình bao `getLocalAABB()` trong không gian cục bộ, còn `getAABB()` sẽ áp dụng `Transform` để trả về hộp bao trong không gian thế giới.

- `Sphere` chỉ lưu bán kính; tâm của nó nằm tại gốc không gian cục bộ `(0, 0, 0)`, còn vị trí trong thế giới được xác định bởi `Transform.translation`.
- `Mesh` quản lý một danh sách `RawTriangle` và một cây BVH (`BVHNode`) để tăng tốc truy vấn giao cắt trên lưới lớn, đồng thời cài đặt thuật toán Moller–Trumbore để kiểm tra giao cắt tia–tam giác và nội suy tọa độ `uv`.
- `RawTriangle` chỉ lưu hình học (một đỉnh gốc, hai vector cạnh và tọa độ kết cấu), không chứa logic giao cắt. Cách lưu này cho phép thuật toán Moller–Trumbore dùng trực tiếp hai cạnh mà không phải tính lại mỗi lần kiểm tra giao cắt.

#figure(
  block(breakable: false, width: 100%)[
    #align(center)[
      #diagram(
        node-shape: rect,
        spacing: (0.2cm, 0.2cm),

        node((-3, 0), class(
          "Matte",
          none,
          (
            "+ Matte(name, texture)",
            "+ scatter(...): ScatterResult",
          )
        ), name: <matte>),

        node((-3, 1), class(
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

        node((3, 0), class(
          "Dielectric",
          none,
          (
            "+ Dielectric(name, texture, medium)",
            "+ scatter(...): ScatterResult",
          )
        ), name: <dielectric>),

        node((3, 1), class(
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

        node((0, 0.5), class(
          "Material",
          (
            "# texture: Texture*",
            "# medium: Medium*",
          ),
          (
            "+ Material(name, texture, medium)",
            "+ scatter(...): ScatterResult",
            "+ emitted(rec): Color",
            "+ getTexture(): Texture*",
            "+ getMedium(): Medium*",
          )
        ), name: <material>),

        node((0, 2), class(
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
          ),
          (
            "+ get(rec): Color = 0",
          )
        ), name: <texture>),

        node((0, 0), class(
          "Material",
          (
            "# texture: Texture*",
            "# medium: Medium*",
          ),
          (
            "+ Material(name, texture, medium)",
            "+ scatter(...): ScatterResult",
            "+ emitted(rec): Color",
            "+ getTexture(): Texture*",
            "+ getMedium(): Medium*",
          )
        ), name: <material>),

        node((1, 0), class(
          "Medium",
          (
            "# ior: float",
          ),
          (
            "+ Medium(name, ior)",
            "+ {virtual} transmit(ray: Ray&, throughput: Color&, distance: const float&): float",
            "+ getIOR(): const float&",
            "+ setIOR(ior: float)",
          )
        ), name: <medium>),

        node((-2, 1.6), class(
          "SolidTexture",
          (
            "# color: Color",
          ),
          (
            "+ SolidTexture(name, color)",
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
            "# sigma_a: Color",
            "# sigma_s: Color",
            "# density: float",
            "# eff_sigma_a: Color",
            "# eff_sigma_s: Color",
            "# eff_sigma_t: Color",
            "# majorant: float",
            "# scattering: bool",
          ),
          (
            "+ Homogeneous(name, ior, sigma_a, sigma_s, density)",
            "+ transmit(ray: Ray&, throughput: Color&, distance: const float&): float",
            "+ transmittance(distance: float): Color",
            "+ getSigmaA(): const Color&",
            "+ setSigmaA(sigma_a: const Color&)",
            "+ getSigmaS(): const Color&",
            "+ setSigmaS(sigma_s: const Color&)",
            "+ getDensity(): float",
            "+ setDensity(density: float)",
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

`Texture` cung cấp dữ liệu bề mặt tại một `HitInfo` bao gồm suy hao bề mặt (màu sắc), độ nhám, pháp tuyến, ...; `SolidTexture` là cài đặt đơn giản nhất, luôn trả về một màu không đổi.

`Medium` mô tả môi trường mà tia đang truyền qua. `Medium::transmit(ray, throughput, distance)` cố gắng đưa tia đi hết một đoạn dài `distance`: hàm nhân `throughput` với suy hao của đoạn đường thực tế và trả về quãng đường đã đi. Nếu giá trị trả về nhỏ hơn `distance`, tia đã bị tán xạ thể tích trước khi tới bề mặt; khi đó `ray` đã được dời tới điểm tán xạ và có hướng mới. Lớp cơ sở `Medium` là môi trường trong suốt nên luôn trả về `distance`.

`Homogeneous` mô tả môi trường đồng nhất với hai hệ số trên một đơn vị mật độ: `sigma_a` (hấp thụ) và `sigma_s` (tán xạ). Các giá trị hiệu dụng `eff_sigma_a = sigma_a * density`, `eff_sigma_s = sigma_s * density` và `eff_sigma_t = eff_sigma_a + eff_sigma_s` được cache lại trong `refreshEffSigma()` để tránh tính lại mỗi đoạn tia. Khi `sigma_s = 0`, môi trường chỉ hấp thụ: `transmit()` dùng định luật Beer–Lambert tất định với `eff_sigma_t` và luôn trả về `distance`, nên không tốn mẫu ngẫu nhiên cho thủy tinh trong suốt. Khi `sigma_s > 0`, môi trường tham gia tán xạ: `transmit()` dùng kênh lớn nhất `majorant = max(eff_sigma_t)` làm tốc độ mẫu, lấy quãng đường bay tự do theo phân phối mũ rồi giới hạn bởi `distance`. Nếu tia tán xạ trước bề mặt, trọng số được nhân thêm `eff_sigma_s / majorant`; phần dư màu do các kênh có `eff_sigma_t` nhỏ hơn `majorant` được đưa vào hệ số `exp(-(eff_sigma_t - majorant) * t)`. Nhờ vậy cùng một lớp `Homogeneous` mô tả được cả thủy tinh (trong suốt hoặc có màu) lẫn khói/sương. Mỗi vật liệu `Dielectric` tham chiếu thêm một `Medium` riêng để mô tả môi trường bên trong nó.

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
          ),
          (
            "+ AABB(p: Vec3 = Vec3(0))",
            "+ AABB(points: initializer_list<Vec3>)",
            "+ extend(p: Vec3&)",
            "+ extend(aabb: AABB&)",
            "+ hit(ray: Ray&, inv_direction: Vec3&, tmin, tmax): bool",
            "+ operator+(p: Vec3&): AABB",
            "+ operator+(aabb: AABB&): AABB",
          )
        ), name: <aabb>),

        node((1, 2.8), class-sm(
          "RawTriangle",
          (
            "# origin, edge1, edge2: Vec3",
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
            "...",
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
            "+ {static} cosineHemisphere(n): Vec3",
            "+ {static} pointInCircle(radius = 1.0f): Vec2",
            "+ {static} vec3(): Vec3",
            "+ {static} vec2(): Vec2",
          )
        ), name: <rng>),

        edge(<vec2>, <vec3>, marks: dependency),
        edge(<color>, <vec3>, marks: dependency, label: [alias]),
        edge(<ray>, <vec3>, marks: composition),
        edge(<aabb>, <vec3>, marks: composition),
        edge(<rawtriangle>, <vec3>, marks: composition),
        edge(<transform>, <mat3x3>, marks: composition),
      )
    ]
  ], caption: [Các kiểu toán học hỗ trợ.]
)

Các kiểu toán học được thiết kế dưới dạng `struct` nhỏ, thao tác trên giá trị và không sở hữu tài nguyên. `Vec3` kiêm luôn vai trò `Color` (với các kênh R, G, B lần lược là các trục x, y, z) và cung cấp các phép toán vector, phản xạ, khúc xạ, chuyển đổi màu sang ABGR. `Ray` và `AABB` là hai kiểu nền tảng cho mọi phép truy vấn giao cắt; `RawTriangle` kết hợp `Vec3` và `Vec2` để lưu hình học và tọa độ kết cấu. `Transform` đóng gói phép biến đổi affine, `Mat3x3` dùng để chứa ma trận xoay, còn `RNG` là lớp tiện ích tĩnh phục vụ lấy mẫu ngẫu nhiên trong mô phỏng Monte Carlo.

=== kết quả

== kết luận và hướng phát triển
=== kết quả đạt được
=== hạn chế
-  *BVH còn đơn giản*. Cây được xây bằng cách chia đôi theo trung vị trên trục dài nhất, mỗi nút lá chứa tối đa 4 đối tượng, chưa dùng heuristic diện tích bề mặt (SAH) nên chất lượng cây chưa tối ưu với các cảnh phân bố không đều.
- *Ổn định số học*. Phép kiểm tra AABB có thể cho NaN khi tia nằm đúng trên mặt slab và song song với trục.
- *Bộ sinh số ngẫu nhiên*. Bộ sinh mt19937 là đối tượng tĩnh dùng chung, trong khi vòng lặp dựng ảnh chạy song song nhiều luồng; việc nhiều luồng cùng truy cập một bộ sinh không an toàn và cần thay bằng bộ sinh riêng cho mỗi luồng.
- *Nhiễu và tốc độ hội tụ*. Ảnh chỉ có một mẫu cho mỗi lượt và ánh sáng chỉ đến từ việc tia ngẫu nhiên tình cờ trúng đèn, nên các cảnh có đèn nhỏ hội tụ chậm. Chưa có lấy mẫu trực tiếp nguồn sáng và chưa có khử nhiễu.
- *Chưa đủ tính năng*. Chỉ có kết cấu màu đơn sắc (chưa có texture từ ảnh), nền cảnh là màu đen (chưa có skybox), cảnh được khai báo trong mã nguồn nên đổi cảnh phải biên dịch lại, chưa có chức năng lưu/tải cảnh và xuất ảnh ra tệp, chưa chỉnh được độ phân giải, số lần dội hay tạm dừng/đặt lại thủ công từ giao diện.
- *Độ sâu trường ảnh và môi trường*. Độ sâu trường ảnh chỉ là xấp xỉ (không có mặt phẳng lấy nét); môi trường đồng nhất chỉ hấp thụ ánh sáng, chưa có tán xạ thể tích.
- *Vật liệu và nền tảng*. Chỉ chạy trên CPU; mô hình vật liệu còn đơn giản, chưa dựa trên BRDF vật lý đầy đủ.
=== hướng phát triển
- Cải thiện BVH bằng SAH và cho phép cập nhật cây khi vật thể di chuyển mà không phải xây lại toàn bộ.
- Dùng bộ sinh số ngẫu nhiên riêng cho mỗi luồng (thread_local) để đảm bảo an toàn đa luồng.
- Bổ sung texture từ ảnh (tận dụng các ảnh đã có sẵn trong thư mục tài nguyên), skybox và ánh sáng môi trường.
- Lưu/tải cảnh từ tệp, đọc mô tả cảnh từ tệp thay vì khai báo trong mã, và xuất ảnh kết quả ra PNG/JPG.
- Giảm nhiễu bằng lấy mẫu trực tiếp nguồn sáng (next event estimation), mở rộng lấy mẫu quan trọng (hiện mới có lấy mẫu cosin cho vật liệu Matte) và bộ khử nhiễu.
- Mở rộng vật liệu theo mô hình vật lý (microfacet), thêm các hình học khác như hình hộp, đĩa, và tán xạ thể tích.
- Chuyển phần tính toán sang GPU (CUDA, OpenCL, hoặc compute shader) để tăng tốc nhiều lần.
=== kết luận
Đề tài đã hoàn thành mục tiêu chính là xây dựng một trình dò tia chạy trên CPU, mô phỏng được các hiện tượng quang học cơ bản và tính toán ánh sáng toàn cục bằng path tracing. Việc áp dụng BVH và đa luồng giúp việc dựng ảnh khả thi với các lưới tam giác lớn, còn giao diện tương tác giúp quan sát ngay ảnh hưởng của mỗi thay đổi. Dù còn một số hạn chế, chương trình có kiến trúc đủ linh hoạt để tiếp tục mở rộng theo các hướng đã nêu.
