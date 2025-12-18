# Physics Theory & Equations

## The Schwarzschild Metric
The line element for the Schwarzschild metric in spherical coordinates $(t, r, \theta, \phi)$ is:

$$c^2 d\tau^2 = \left(1 - \frac{r_s}{r}\right) c^2 dt^2 - \left(1 - \frac{r_s}{r}\right)^{-1} dr^2 - r^2 d\Omega^2$$

Where:
- $r_s$ is the Schwarzschild radius: $r_s = \frac{2GM}{c^2}$
- $d\Omega^2 = d\theta^2 + \sin^2\theta d\phi^2$

## Geodesic Equations
Light travels along null geodesics ($d\tau = 0$). To simulate this path, we solve the geodesic equations involved. The effective potential for a photon orbiting a black hole is:

$$V_{eff}(r) = \frac{L^2}{2r^2} \left(1 - \frac{r_s}{r}\right)$$

Where $L$ is the angular momentum.

## Numerical Integration (RK4)
We cannot solve the path of every photon analytically in real-time for an entire image. Instead, we use numerical integration. Specifically, the **Runge-Kutta 4th Order (RK4)** method.

Given the state vector $\mathbf{y} = (r, \phi, \dot{r}, \dot{\phi})$, we compute the next step $\mathbf{y}_{n+1}$:

$$
\begin{aligned}
k_1 &= f(t_n, \mathbf{y}_n) \\
k_2 &= f(t_n + \frac{h}{2}, \mathbf{y}_n + h \frac{k_1}{2}) \\
k_3 &= f(t_n + \frac{h}{2}, \mathbf{y}_n + h \frac{k_2}{2}) \\
k_4 &= f(t_n + h, \mathbf{y}_n + h k_3) \\
\mathbf{y}_{n+1} &= \mathbf{y}_n + \frac{h}{6}(k_1 + 2k_2 + 2k_3 + k_4)
\end{aligned}
$$

The equations of motion derived from the Lagrangian for the Schwarzschild metric are used to define the function $f$ in our shader code.

## 2D Lensing Approximation
For the 2D lensing demo (`2D_lensing.cpp`), we simplify the problem to the equatorial plane ($\theta = \pi/2$). The deflection angle $\alpha$ for a light ray with impact parameter $b$ passing a mass $M$ in the weak field limit is:

$$\alpha = \frac{4GM}{bc^2}$$

However, our simulation uses the full geodesic integration even for the 2D view to remain accurate near the event horizon.
