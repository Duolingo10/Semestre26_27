%% PRACTICA 5 - Modelo, Routh modificado, Kp y simulacion
% Requiere Control System Toolbox para tf, feedback, lsim y pole.
clear; clc; close all;

%% 1. Escribe los coeficientes obtenidos en tu Practica 4
h = 0.010;                         % Periodo de muestreo: 10 ms
a = NaN;                          % <-- REEMPLAZAR por tu valor de a
b = NaN;                          % <-- REEMPLAZAR por tu valor de b

if ~isfinite(a) || ~isfinite(b) || a <= 0
    error('Escribe tus valores reales de a y b. Este codigo supone a > 0.');
end

%% 2. Intervalo estable: |b - a*Kp| < 1
Kmin = max(0, (b - 1)/a);           % Solo ganancias positivas
Kmax = (b + 1)/a;
if Kmax <= Kmin
    error('No hay una ganancia Kp positiva estable para estos coeficientes.');
end
fprintf('G(z) = %.8g/(z - %.8g)\n', a, b);
fprintf('Intervalo de estabilidad con Kp > 0: %.6f < Kp < %.6f\n', Kmin, Kmax);

% Seleccion conservadora: evita saturar el DAC a partir de error = 2.5 V
Kcap = 1.9;                        % 2.5*1.9 = 4.75 V al inicio
if Kmin < min(Kmax, Kcap)
    Kp = (Kmin + min(Kmax, Kcap))/2;
else
    Kp = (Kmin + Kmax)/2;
    warning('La Kp estable puede saturar el DAC: la simulacion lineal diferira del motor.');
end
fprintf('Kp propuesta: %.6f\n', Kp);
fprintf('En el Arduino pon: const float Kp = %.6ff;\n', Kp);

%% 3. Funcion de transferencia y lazo cerrado
Gz = tf(a, [1 -b], h, 'Variable', 'z');
Gc = feedback(Kp * Gz, 1);
disp('Planta G(z):'); disp(Gz);
disp('Lazo cerrado Gc(z):'); disp(Gc);
fprintf('Polo de lazo cerrado: %.6f\n', b - a*Kp);

%% 4. Teorema del valor final, referencia escalon 2.5 V
yss = 2.5 * (a*Kp)/(1 - b + a*Kp);
fprintf('Salida teorica estacionaria yss = %.6f V\n', yss);

%% 5. Simulacion de escalon de 2.5 V
t = (0:h:8)';
r = 2.5 * ones(size(t));
y = lsim(Gc, r, t);
figure('Name','Practica 5 - Escalon');
plot(t, r, '--', 'LineWidth', 1.4); hold on;
plot(t, y, 'LineWidth', 1.4);
yline(yss, ':', 'yss teorica', 'LineWidth', 1.2);
grid on; xlabel('Tiempo (s)'); ylabel('Voltaje (V)');
title('Sistema con control proporcional: escalon de 2.5 V');
legend('Referencia','Salida simulada','Valor final','Location','best');

%% 6. Simulacion de referencia 1 Hz, ciclo 50 %, 0 a 2.5 V
rCuadrada = 2.5 * double(mod(t, 1) < 0.5);
yCuadrada = lsim(Gc, rCuadrada, t);
figure('Name','Practica 5 - Referencia cuadrada');
plot(t, rCuadrada, '--', 'LineWidth', 1.2); hold on;
plot(t, yCuadrada, 'LineWidth', 1.2);
grid on; xlabel('Tiempo (s)'); ylabel('Voltaje (V)');
title('Simulacion en lazo cerrado: 1 Hz, ciclo 50 %');
legend('Referencia','Salida simulada','Location','best');
