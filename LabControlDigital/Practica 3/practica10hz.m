clear; clc;

Fs = 10; % <--- AQUÍ ES LO ÚNICO QUE CAMBIAS (10, 20 o 100)
Ts = 1/Fs;
duracion = 3000; % Captura de 3 segundos

% Valores nominales
R = 1000;
L = 100e-6;
C = 10e-6;

% --- 1. CAPTURA DE DATOS REALES ---
s = serialport("COM7", 9600); % <--- ASEGÚRATE DE QUE SEA TU PUERTO COM
configureTerminator(s, "CR/LF");
flush(s);

nMuestras = duracion/(1000/Fs) + 10;
k = zeros(1, nMuestras);
yk = zeros(1, nMuestras);
cnt = -1;
pause(3);

disp('Iniciando captura... ¡Conecta el cable de 5V a la resistencia AHORA!');

while cnt <= nMuestras
    if (cnt > 0)
        dataStr = readline(s);
        if strlength(dataStr) > 0 
            aRead = sscanf(dataStr, '%d,%d'); 
            if length(aRead) == 2
                k(cnt) = aRead(1);
                yk(cnt) = aRead(2)*5/1023;
            end
        end
    end
    cnt = cnt + 1;
end
clear s;

% Limpiar basura inicial de la lectura serial
[m,n] = size(k);
k = k(11:n)/1000;
yk = yk(11:n);

% --- 3. RECORTE DE LA REGIÓN DE INTERÉS (TRANSITORIO) ---
indice_salto = find(yk > 0.2, 1, 'first');

if ~isempty(indice_salto)
    % Tomar 2 muestras antes y el equivalente a 1 segundo después
    inicio = max(1, indice_salto - 2);
    fin = min(length(yk), indice_salto + round(1.5 / Ts)); 

    k_recortado = k(inicio:fin);
    yk_recortado = yk(inicio:fin);
    k_recortado = k_recortado - k_recortado(1);
else
    k_recortado = k - k(1);
    yk_recortado = yk;
end

% --- 4. MODELO TEÓRICO Y DISCRETIZACIÓN ---
num = 1;
den = [L*C, R*C, 1];
Gs = tf(num, den);
Gz = c2d(Gs, Ts, 'zoh');
[y_sim_recortada, t_sim_recortada] = step(5 * Gz, k_recortado(end));

% --- 5. GRÁFICA FINAL RECORTADA ---
figure(1)
plot(k_recortado, yk_recortado, 'b-', 'LineWidth', 2); hold on;
stairs(t_sim_recortada, y_sim_recortada, 'r--', 'LineWidth', 2);
title(['Región de Interés (Transitorio) - Fs = ', num2str(Fs), ' Hz']); 
grid on;
xlabel('Tiempo (s)'); 
ylabel('Voltaje Vc (V)');
legend('Datos Reales (Transitorio)', 'Modelo Discreto ZOH', 'Location', 'best');
hold off;