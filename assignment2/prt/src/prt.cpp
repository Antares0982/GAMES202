#include <Eigen/Core>
#include <filesystem/resolver.h>
#include <fstream>
#include <nori/integrator.h>
#include <nori/ray.h>
#include <nori/scene.h>
#include <random>
#include <sh/default_image.h>
#include <sh/spherical_harmonics.h>
#include <stb_image.h>

NORI_NAMESPACE_BEGIN

namespace ProjEnv
{
    std::vector<std::unique_ptr<float[]>>
    LoadCubemapImages(const std::string &cubemapDir, int &width, int &height,
                      int &channel)
    {
        std::vector<std::string> cubemapNames{"negx.jpg", "posx.jpg", "posy.jpg",
                                              "negy.jpg", "posz.jpg", "negz.jpg"};
        std::vector<std::unique_ptr<float[]>> images(6);
        for (int i = 0; i < 6; i++)
        {
            std::string filename = cubemapDir + "/" + cubemapNames[i];
            int w, h, c;
            float *image = stbi_loadf(filename.c_str(), &w, &h, &c, 3);
            if (!image)
            {
                std::cout << "Failed to load image: " << filename << std::endl;
                exit(-1);
            }
            if (i == 0)
            {
                width = w;
                height = h;
                channel = c;
            }
            else if (w != width || h != height || c != channel)
            {
                std::cout << "Dismatch resolution for 6 images in cubemap" << std::endl;
                exit(-1);
            }
            images[i] = std::unique_ptr<float[]>(image);
            int index = (0 * 128 + 0) * channel;
            // std::cout << images[i][index + 0] << "\t" << images[i][index + 1] << "\t"
            //           << images[i][index + 2] << std::endl;
        }
        return images;
    }

    const Eigen::Vector3f cubemapFaceDirections[6][3] = {
        {{0, 0, 1}, {0, -1, 0}, {-1, 0, 0}},  // negx
        {{0, 0, 1}, {0, -1, 0}, {1, 0, 0}},   // posx
        {{1, 0, 0}, {0, 0, -1}, {0, -1, 0}},  // negy
        {{1, 0, 0}, {0, 0, 1}, {0, 1, 0}},    // posy
        {{-1, 0, 0}, {0, -1, 0}, {0, 0, -1}}, // negz
        {{1, 0, 0}, {0, -1, 0}, {0, 0, 1}},   // posz
    };

    float CalcPreArea(const float &x, const float &y)
    {
        return std::atan2(x * y, std::sqrt(x * x + y * y + 1.0));
    }

    float CalcArea(const float &u_, const float &v_, const int &width,
                   const int &height)
    {
        // transform from [0..res - 1] to [- (1 - 1 / res) .. (1 - 1 / res)]
        // ( 0.5 is for texel center addressing)
        float u = (2.0 * (u_ + 0.5) / width) - 1.0;
        float v = (2.0 * (v_ + 0.5) / height) - 1.0;

        // shift from a demi texel, mean 1.0 / size  with u and v in [-1..1]
        float invResolutionW = 1.0 / width;
        float invResolutionH = 1.0 / height;

        // u and v are the -1..1 texture coordinate on the current face.
        // get projected area for this texel
        float x0 = u - invResolutionW;
        float y0 = v - invResolutionH;
        float x1 = u + invResolutionW;
        float y1 = v + invResolutionH;
        float angle = CalcPreArea(x0, y0) - CalcPreArea(x0, y1) -
                      CalcPreArea(x1, y0) + CalcPreArea(x1, y1);

        return angle;
    }

    // template <typename T> T ProjectSH() {}

    template <size_t SHOrder>
    std::vector<Eigen::Array3f> PrecomputeCubemapSH(const std::vector<std::unique_ptr<float[]>> &images,
                                                    const int &width, const int &height,
                                                    const int &channel)
    {
        /// cubemapDirs，保存了cubemap 6张贴图上每个像素对应的单位向量
        std::vector<Eigen::Vector3f> cubemapDirs;
        cubemapDirs.reserve(6 * width * height);

        /// 写入cubemapDirs
        for (int i = 0; i < 6; i++)
        {
            Eigen::Vector3f faceDirX = cubemapFaceDirections[i][0];
            Eigen::Vector3f faceDirY = cubemapFaceDirections[i][1];
            Eigen::Vector3f faceDirZ = cubemapFaceDirections[i][2];
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    float u = 2 * ((x + 0.5) / width) - 1;
                    float v = 2 * ((y + 0.5) / height) - 1;
                    // Eigen::Vector3f dir = (faceDirX * u + faceDirY * v + faceDirZ).normalized();
                    // cubemapDirs.push_back(dir);
                    cubemapDirs.emplace_back((faceDirX * u + faceDirY * v + faceDirZ).normalized());
                }
            }
        }

        constexpr int SHNum = (SHOrder + 1) * (SHOrder + 1);

        //        std::vector<Eigen::Array3f> SHCoeffiecents(SHNum);
        //        for (int i = 0; i < SHNum; i++)
        //            SHCoeffiecents[i] = Eigen::Array3f(0);
        std::vector<Eigen::Array3f> SHCoeffiecents(SHNum, Eigen::Array3f(0));
        /// fill up SHCoeffiecents

        /// unused?
        // float sumWeight = 0;
        for (int i = 0; i < 6; i++)
        {
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    // TODO: here you need to compute light sh of each face of cubemap of each pixel
                    // TODO: 此处你需要计算每个像素下cubemap某个面的球谐系数

                    /// Note: requiring double in EvalSH()
                    const Eigen::Vector3d dir = cubemapDirs[i * width * height + y * width + x].cast<double>().normalized();
                    int index = (y * width + x) * channel;
                    /// L_env, environment light
                    Eigen::Array3f Le(images[i][index + 0], images[i][index + 1], images[i][index + 2]);

                    /// finish code here
                    for (int level = 0; level <= SHOrder; ++level)
                    {
                        for (int m = -level; m <= level; ++m)
                        {
                            SHCoeffiecents[sh::GetIndex(level, m)] += Le * sh::EvalSH(level, m, dir) * CalcArea(x, y, width, height);
                        }
                    }
                }
            }
        }
        return SHCoeffiecents;
    }
} // namespace ProjEnv

class PRTIntegrator : public Integrator
{
public: // static
    static constexpr int SHOrder = 2;

    static constexpr int SHCoeffLength = (SHOrder + 1) * (SHOrder + 1);

public: // class definition
    enum class Type
    {
        Unshadowed = 0,
        Shadowed = 1,
        Interreflection = 2
    };

public: // constructor
    PRTIntegrator(const PropertyList &props)
    {
        /* No parameters this time */
        m_SampleCount = props.getInteger("PRTSampleCount", 100);
        m_CubemapPath = props.getString("cubemap");
        auto type = props.getString("type", "unshadowed");
        if (type == "unshadowed")
        {
            m_Type = Type::Unshadowed;
        }
        else if (type == "shadowed")
        {
            m_Type = Type::Shadowed;
        }
        else if (type == "interreflection")
        {
            m_Type = Type::Interreflection;
            m_Bounce = props.getInteger("bounce", 1);
        }
        else
        {
            throw NoriException("Unsupported type: %s.", type);
        }
    }

public: // virtual func
    void preprocess(const Scene *scene) override
    {
        // Here only compute one mesh
        const auto mesh = scene->getMeshes()[0];
        // Projection environment
        /// get cubePath (file path)
        auto cubePath = getFileResolver()->resolve(m_CubemapPath);

        int width, height, channel;
        std::vector<std::unique_ptr<float[]>> images = ProjEnv::LoadCubemapImages(cubePath.str(), width, height, channel);

        /// write
        auto lightPath = cubePath / "light.txt";
        auto transPath = cubePath / "transport.txt";

        std::ofstream lightFout(lightPath.str());
        std::ofstream fout(transPath.str());

        /// get pre-computed SH coeffs
        auto envCoeffs = ProjEnv::PrecomputeCubemapSH<SHOrder>(images, width, height, channel);

        m_LightCoeffs.resize(3, SHCoeffLength);
        for (int i = 0; i < envCoeffs.size(); i++)
        {
            lightFout << (envCoeffs)[i].x() << " " << (envCoeffs)[i].y() << " " << (envCoeffs)[i].z() << std::endl;
            m_LightCoeffs.col(i) = (envCoeffs)[i];
        }
        std::cout << "Computed light sh coeffs from: " << cubePath.str() << " to: " << lightPath.str() << std::endl;

        // Projection transport
        /// (SHOrder+1)^2 rows, VertexCount cols
        /// SHCoeffLength = (SHOrder+1)^2
        m_TransportSHCoeffs.resize(SHCoeffLength, mesh->getVertexCount());
        /// write in file
        fout << mesh->getVertexCount() << std::endl;

        for (int i = 0; i < mesh->getVertexCount(); i++)
        {
            const Point3f &v = mesh->getVertexPositions().col(i);
            const Normal3f &n = mesh->getVertexNormals().col(i);

            /// input: w_i, output: L(w_i) * max(0, cos(\theta)).
            auto shFunc = [&](double phi, double theta) -> double
            {
                Eigen::Array3d d = sh::ToVector(phi, theta);

                /// sample vector
                const auto wi = Vector3f(d.x(), d.y(), d.z());

                double H = wi.dot(n);

                if (H < 0.0)
                    return 0.0;
                if (m_Type == Type::Unshadowed)
                {
                    return H;
                    // TODO: here you need to calculate unshadowed transport term of a given direction
                    // TODO: 此处你需要计算给定方向下的unshadowed传输项球谐函数值
                    // return 0;
                }
                else
                {
                    // test if there should be shadow
                    return (scene->rayIntersect(Ray3f(v, wi))) ? 0.0 : H;

                    // TODO: here you need to calculate shadowed transport term of a given direction
                    // TODO: 此处你需要计算给定方向下的shadowed传输项球谐函数值
                    // return 0;
                }
            };

            /// lambda is passed to sh::ProjectFunction
            /// sh::ProjectFunction returns std::vector<double>
            auto shCoeff = sh::ProjectFunction(SHOrder, shFunc, m_SampleCount);
            for (int j = 0; j < shCoeff->size(); j++)
            {
                m_TransportSHCoeffs.col(i).coeffRef(j) = (*shCoeff)[j];
            }
        }
        if (m_Type == Type::Interreflection)
        {
            Eigen::MatrixXf interreflectionCoefs;
            interreflectionCoefs.resize(SHCoeffLength, mesh->getVertexCount());

            for (int i = 0; i < mesh->getVertexCount(); ++i)
            {
                bool vis = false;
                for (int j = 0; j < SHCoeffLength; ++j)
                {
                    if (m_TransportSHCoeffs.col(i).coeffRef(j) != 0)
                    {
                        vis = true;
                        break;
                    }
                }
                if (vis)
                    continue;
                const Point3f v = mesh->getVertexPositions().col(i);
                const Normal3f n = mesh->getVertexNormals().col(i);
                auto shFunc = [&](double phi, double theta) -> std::vector<double>
                {
                    Eigen::Array3d d = sh::ToVector(phi, theta);

                    /// sample vector
                    const auto wi = Vector3f(d.x(), d.y(), d.z());

                    double H = wi.dot(n);
                    std::vector<double> ans(SHCoeffLength, 0.0);
                    if (H < 0.0)
                        return ans;
                    Intersection it;
                    bool intersect = scene->rayIntersect(Ray3f(v, wi), it);
                    if (!intersect)
                        return ans;
                    auto a = m_TransportSHCoeffs.col(it.tri_index.x());
                    auto b = m_TransportSHCoeffs.col(it.tri_index.y());
                    auto c = m_TransportSHCoeffs.col(it.tri_index.z());
                    auto x = it.bary.x();
                    auto y = it.bary.y();
                    auto z = it.bary.z();
                    auto result = (a * x + b * y + c * z) * H;
                    for (int i = 0; i < ans.size(); ++i)
                    {
                        ans[i] = result[i];
                    }
                    return ans;
                };

                const int sample_side = static_cast<int>(floor(sqrt(m_SampleCount)));
                // auto shCoeff = sh::ProjectFunction(SHOrder, shFunc, m_SampleCount);

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> rng(0.0, 1.0);
                for (int t = 0; t < sample_side; t++)
                {
                    for (int p = 0; p < sample_side; p++)
                    {
                        double alpha = (t + rng(gen)) / sample_side;
                        double beta = (p + rng(gen)) / sample_side;
                        // See http://www.bogotobogo.com/Algorithms/uniform_distribution_sphere.php
                        double phi = 2.0 * M_PI * beta;
                        double theta = acos(2.0 * alpha - 1.0);

                        // evaluate the analytic function for the current spherical coords
                        auto func_value = shFunc(phi, theta);

                        // evaluate the SH basis functions up to band O, scale them by the
                        // function's value and accumulate them over all generated samples
                        for (int j = 0; j < SHCoeffLength; ++j)
                        {
                            interreflectionCoefs.col(i).coeffRef(j) += func_value[j];
                        }
                    }
                }

                // scale by the probability of a particular sample, which is
                // 4pi/sample_side^2. 4pi for the surface area of a unit sphere, and
                // 1/sample_side^2 for the number of samples drawn uniformly.
                interreflectionCoefs.col(i) *= 4.0 * M_PI / (sample_side * sample_side);
            }
            // TODO: leave for bonus

            for (int i = 0; i < mesh->getVertexCount(); ++i)
            {
                m_TransportSHCoeffs.col(i) += interreflectionCoefs.col(i);
            }
        }

        // Save in face format
        for (int f = 0; f < mesh->getTriangleCount(); f++)
        {
            const MatrixXu &F = mesh->getIndices();
            uint32_t idx0 = F(0, f), idx1 = F(1, f), idx2 = F(2, f);
            for (int j = 0; j < SHCoeffLength; j++)
            {
                fout << m_TransportSHCoeffs.col(idx0).coeff(j) << " ";
            }
            fout << std::endl;
            for (int j = 0; j < SHCoeffLength; j++)
            {
                fout << m_TransportSHCoeffs.col(idx1).coeff(j) << " ";
            }
            fout << std::endl;
            for (int j = 0; j < SHCoeffLength; j++)
            {
                fout << m_TransportSHCoeffs.col(idx2).coeff(j) << " ";
            }
            fout << std::endl;
        }
        std::cout << "Computed SH coeffs"
                  << " to: " << transPath.str() << std::endl;
    }

public: // member func
    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const
    {
        Intersection its;
        if (!scene->rayIntersect(ray, its))
            return Color3f(0.0f);

        const Eigen::Matrix<Vector3f::Scalar, SHCoeffLength, 1> sh0 = m_TransportSHCoeffs.col(its.tri_index.x()),
                                                                sh1 = m_TransportSHCoeffs.col(its.tri_index.y()),
                                                                sh2 = m_TransportSHCoeffs.col(its.tri_index.z());
        const Eigen::Matrix<Vector3f::Scalar, SHCoeffLength, 1> rL = m_LightCoeffs.row(0), gL = m_LightCoeffs.row(1), bL = m_LightCoeffs.row(2);

        Color3f c0 = Color3f(rL.dot(sh0), gL.dot(sh0), bL.dot(sh0)),
                c1 = Color3f(rL.dot(sh1), gL.dot(sh1), bL.dot(sh1)),
                c2 = Color3f(rL.dot(sh2), gL.dot(sh2), bL.dot(sh2));

        const Vector3f &bary = its.bary;
        Color3f c = bary.x() * c0 + bary.y() * c1 + bary.z() * c2;
        // TODO: you need to delete the following four line codes after finishing your calculation to SH,
        //       we use it to visualize the normals of model for debug.
        // TODO: 在完成了球谐系数计算后，你需要删除下列四行，这四行代码的作用是用来可视化模型法线
        // if (c.isZero()) {
        //     auto n_ = its.shFrame.n.cwiseAbs();
        //     return Color3f(n_.x(), n_.y(), n_.z());
        // }
        return c;
    }

    std::string toString() const
    {
        return "PRTIntegrator[]";
    }

private:
    Type m_Type;
    int m_Bounce = 1;
    int m_SampleCount = 100;
    std::string m_CubemapPath;
    Eigen::MatrixXf m_TransportSHCoeffs;
    Eigen::MatrixXf m_LightCoeffs;
};

NORI_REGISTER_CLASS(PRTIntegrator, "prt");
NORI_NAMESPACE_END
