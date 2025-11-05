#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <cmath>
#include "logger.hpp"

/**
 * Ray Path Exporter
 * Exports geodesic paths to CSV for scientific analysis
 */
class RayPathExporter {
private:
    struct PathPoint {
        double r, theta, phi;           // Spherical coordinates
        double x, y, z;                 // Cartesian coordinates
        double dr, dtheta, dphi;        // Velocities
        double E, L;                    // Conserved quantities
        double lambda;                  // Affine parameter
    };

    const double c = 299792458.0;
    const double G = 6.67430e-11;
    const double SagA_rs = 1.269e10;    // Schwarzschild radius
    const double D_LAMBDA = 1e7;        // Integration step
    const int MAX_STEPS = 60000;
    const double ESCAPE_R = 1e30;

    void geodesicRHS(const PathPoint& point, glm::dvec3& d1, glm::dvec3& d2) {
        double r = point.r;
        double theta = point.theta;
        double dr = point.dr;
        double dtheta = point.dtheta;
        double dphi = point.dphi;

        double f = 1.0 - SagA_rs / r;
        double dt_dL = point.E / f;

        d1 = glm::dvec3(dr, dtheta, dphi);

        // Schwarzschild geodesic equations
        d2.x = -(SagA_rs / (2.0 * r*r)) * f * dt_dL * dt_dL
             + (SagA_rs / (2.0 * r*r * f)) * dr * dr
             + r * (dtheta*dtheta + sin(theta)*sin(theta)*dphi*dphi);

        d2.y = -2.0*dr*dtheta/r + sin(theta)*cos(theta)*dphi*dphi;

        d2.z = -2.0*dr*dphi/r - 2.0*cos(theta)/sin(theta) * dtheta * dphi;
    }

    PathPoint rk4Step(const PathPoint& p0, double dL) {
        PathPoint result = p0;

        // k1
        glm::dvec3 k1a, k1b;
        geodesicRHS(p0, k1a, k1b);

        // k2
        PathPoint p2 = p0;
        p2.r = p0.r + 0.5 * dL * k1a.x;
        p2.theta = p0.theta + 0.5 * dL * k1a.y;
        p2.phi = p0.phi + 0.5 * dL * k1a.z;
        p2.dr = p0.dr + 0.5 * dL * k1b.x;
        p2.dtheta = p0.dtheta + 0.5 * dL * k1b.y;
        p2.dphi = p0.dphi + 0.5 * dL * k1b.z;
        glm::dvec3 k2a, k2b;
        geodesicRHS(p2, k2a, k2b);

        // k3
        PathPoint p3 = p0;
        p3.r = p0.r + 0.5 * dL * k2a.x;
        p3.theta = p0.theta + 0.5 * dL * k2a.y;
        p3.phi = p0.phi + 0.5 * dL * k2a.z;
        p3.dr = p0.dr + 0.5 * dL * k2b.x;
        p3.dtheta = p0.dtheta + 0.5 * dL * k2b.y;
        p3.dphi = p0.dphi + 0.5 * dL * k2b.z;
        glm::dvec3 k3a, k3b;
        geodesicRHS(p3, k3a, k3b);

        // k4
        PathPoint p4 = p0;
        p4.r = p0.r + dL * k3a.x;
        p4.theta = p0.theta + dL * k3a.y;
        p4.phi = p0.phi + dL * k3a.z;
        p4.dr = p0.dr + dL * k3b.x;
        p4.dtheta = p0.dtheta + dL * k3b.y;
        p4.dphi = p0.dphi + dL * k3b.z;
        glm::dvec3 k4a, k4b;
        geodesicRHS(p4, k4a, k4b);

        // Weighted average
        result.r = p0.r + (dL/6.0) * (k1a.x + 2.0*k2a.x + 2.0*k3a.x + k4a.x);
        result.theta = p0.theta + (dL/6.0) * (k1a.y + 2.0*k2a.y + 2.0*k3a.y + k4a.y);
        result.phi = p0.phi + (dL/6.0) * (k1a.z + 2.0*k2a.z + 2.0*k3a.z + k4a.z);
        result.dr = p0.dr + (dL/6.0) * (k1b.x + 2.0*k2b.x + 2.0*k3b.x + k4b.x);
        result.dtheta = p0.dtheta + (dL/6.0) * (k1b.y + 2.0*k2b.y + 2.0*k3b.y + k4b.y);
        result.dphi = p0.dphi + (dL/6.0) * (k1b.z + 2.0*k2b.z + 2.0*k3b.z + k4b.z);

        // Update Cartesian coordinates
        result.x = result.r * sin(result.theta) * cos(result.phi);
        result.y = result.r * sin(result.theta) * sin(result.phi);
        result.z = result.r * cos(result.theta);

        result.lambda = p0.lambda + dL;
        result.E = p0.E;
        result.L = p0.L;

        return result;
    }

    PathPoint initializeRay(const glm::vec3& pos, const glm::vec3& dir) {
        PathPoint point;

        // Convert to spherical coordinates
        point.r = glm::length(glm::dvec3(pos));
        point.theta = acos(pos.z / point.r);
        point.phi = atan2(pos.y, pos.x);

        // Convert direction to spherical velocities
        point.dr = sin(point.theta)*cos(point.phi)*dir.x +
                   sin(point.theta)*sin(point.phi)*dir.y +
                   cos(point.theta)*dir.z;

        point.dtheta = (cos(point.theta)*cos(point.phi)*dir.x +
                       cos(point.theta)*sin(point.phi)*dir.y -
                       sin(point.theta)*dir.z) / point.r;

        point.dphi = (-sin(point.phi)*dir.x + cos(point.phi)*dir.y) /
                     (point.r * sin(point.theta));

        // Compute conserved quantities
        point.L = point.r * point.r * sin(point.theta) * point.dphi;
        double f = 1.0 - SagA_rs / point.r;
        double dt_dL = sqrt((point.dr*point.dr)/f +
                           point.r*point.r*(point.dtheta*point.dtheta +
                           sin(point.theta)*sin(point.theta)*point.dphi*point.dphi));
        point.E = f * dt_dL;

        point.x = pos.x;
        point.y = pos.y;
        point.z = pos.z;
        point.lambda = 0.0;

        return point;
    }

public:
    /**
     * Export a ray path to CSV file
     * @param cameraPos Camera position in world space
     * @param cameraDir Camera forward direction (normalized)
     * @param filename Output CSV filename
     * @return true if successful
     */
    bool exportPath(const glm::vec3& cameraPos, const glm::vec3& cameraDir, const std::string& filename) {
        Logger::info("Exporting ray path to ", filename);

        std::vector<PathPoint> path;
        PathPoint current = initializeRay(cameraPos, cameraDir);
        path.push_back(current);

        // Integrate geodesic
        bool hitEventHorizon = false;
        for (int step = 0; step < MAX_STEPS; step++) {
            if (current.r <= SagA_rs) {
                hitEventHorizon = true;
                break;
            }

            if (current.r > ESCAPE_R) {
                break;  // Escaped to infinity
            }

            current = rk4Step(current, D_LAMBDA);
            path.push_back(current);
        }

        // Write to CSV
        std::ofstream file(filename);
        if (!file.is_open()) {
            Logger::error("Failed to open ", filename, " for writing");
            return false;
        }

        // Header
        file << "lambda,r,theta,phi,x,y,z,dr,dtheta,dphi,E,L\n";

        // Data
        for (const auto& point : path) {
            file << point.lambda << ","
                 << point.r << ","
                 << point.theta << ","
                 << point.phi << ","
                 << point.x << ","
                 << point.y << ","
                 << point.z << ","
                 << point.dr << ","
                 << point.dtheta << ","
                 << point.dphi << ","
                 << point.E << ","
                 << point.L << "\n";
        }

        file.close();

        Logger::info("Exported ", path.size(), " points. ",
                    (hitEventHorizon ? "Ray hit event horizon" : "Ray escaped"));

        return true;
    }

    /**
     * Export multiple rays in a cone pattern for visualization
     */
    bool exportConePattern(const glm::vec3& cameraPos, const glm::vec3& cameraDir,
                          int numRays, float angleSpread, const std::string& filename) {
        Logger::info("Exporting ", numRays, " ray paths to ", filename);

        std::ofstream file(filename);
        if (!file.is_open()) {
            Logger::error("Failed to open ", filename, " for writing");
            return false;
        }

        // Header with ray_id
        file << "ray_id,lambda,r,theta,phi,x,y,z\n";

        // Generate rays in cone pattern
        for (int rayId = 0; rayId < numRays; rayId++) {
            // Generate direction within cone
            float angle = angleSpread * (rayId - numRays/2.0f) / (numRays/2.0f);
            glm::vec3 right = glm::normalize(glm::cross(cameraDir, glm::vec3(0, 1, 0)));
            glm::vec3 rayDir = glm::normalize(cameraDir + right * (float)tan(angle));

            PathPoint current = initializeRay(cameraPos, rayDir);

            // Integrate this ray
            for (int step = 0; step < MAX_STEPS; step++) {
                file << rayId << ","
                     << current.lambda << ","
                     << current.r << ","
                     << current.theta << ","
                     << current.phi << ","
                     << current.x << ","
                     << current.y << ","
                     << current.z << "\n";

                if (current.r <= SagA_rs || current.r > ESCAPE_R) {
                    break;
                }

                current = rk4Step(current, D_LAMBDA * 10);  // Larger steps for visualization
            }
        }

        file.close();
        Logger::info("Cone pattern export complete");
        return true;
    }
};
