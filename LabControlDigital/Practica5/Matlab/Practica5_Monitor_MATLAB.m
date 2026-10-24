%% PRACTICA 5 - Captura de datos reales enviados por Arduino UNO R4
% No requiere Arduino Support Package; utiliza serialport de MATLAB.
clear; clc; close all;

%% 1. Configura la captura
puerto = "COM5";                   % <-- CAMBIAR segun serialportlist("available")
duracion = 20;                     % Segundos de captura
modo = "sin_perturbacion";          % Cambiar a "con_perturbacion" para repetir

%% 2. Abre el puerto serial del Arduino
s = serialport(puerto, 115200);
configureTerminator(s, "LF");
s.Timeout = 2;
pause(2);                          % Espera al posible reinicio del Arduino
flush(s);

%% 3. Recibe: tiempo, referencia, salida, control, error
Nmax = ceil(duracion * 150);
datos = nan(Nmax, 5);
n = 0;
inicio = tic;
while toc(inicio) < duracion && n < Nmax
    linea = readline(s);
    if isempty(linea)
        continue;
    end
    valores = sscanf(char(linea), '%f,%f,%f,%f,%f');
    if numel(valores) == 5 && all(isfinite(valores))
        n = n + 1;
        datos(n,:) = valores(:).';
    end
end
clear s;                           % Libera COM para otras aplicaciones

if n < 5
    error('Muy pocas lecturas. Revisa el puerto, baudrate y codigo Arduino.');
end

datos = datos(1:n,:);
t = datos(:,1) - datos(1,1);
r = datos(:,2);
y = datos(:,3);
u = datos(:,4);
e = datos(:,5);

%% 4. Genera las graficas que solicita la practica
fig = figure('Name', 'Practica 5 - Datos reales');
tiledlayout(3,1);
nexttile;
plot(t, r, '--', t, y, '-', 'LineWidth', 1.1);
grid on; ylabel('V'); title('Referencia r(t) y salida v_s(t)');
legend('r(t)','v_s(t)','Location','best');
nexttile;
plot(t, r, '--', t, u, '-', 'LineWidth', 1.1);
grid on; ylabel('V'); title('Referencia r(t) y control v_i(t)');
legend('r(t)','v_i(t)','Location','best');
nexttile;
plot(t, r, '--', t, e, '-', 'LineWidth', 1.1);
grid on; xlabel('Tiempo (s)'); ylabel('V');
title('Referencia r(t) y error e(t)');
legend('r(t)','e(t)','Location','best');

%% 5. Guarda datos y figura para el reporte
T = table(t, r, y, u, e, 'VariableNames', ...
    {'Tiempo_s','Referencia_V','Salida_V','Control_V','Error_V'});
writetable(T, 'Practica5_' + modo + '.csv');
exportgraphics(fig, 'Practica5_' + modo + '.png', 'Resolution', 160);
fprintf('Captura terminada: %d muestras guardadas.\n', n);
