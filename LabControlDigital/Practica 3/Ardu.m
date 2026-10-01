% Conectar al puerto serie (Verifica que sea COM7)
puerto = serialport("COM7", 9600, "Timeout", 5); 
configureTerminator(puerto, "CR/LF");

disp('Escuchando al Arduino... Presiona Ctrl+C para detener.');

while true
    % Leer el dato entrante
    dato_texto = readline(puerto); 

    % Convertir el dato a valor numérico
    distancia_cm = str2double(dato_texto);

    % Imprimir el dato solo si la conversión fue exitosa
    if ~isnan(distancia_cm)
        fprintf('Distancia detectada: %.1f cm\n', distancia_cm);
    end
end