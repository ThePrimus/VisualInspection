Benötigt werden OpenCV 3.1, IDS uEye SDK, IDS UI-1460SE-C-HG Treiber und Visual Studio 15. Zusätzlich wird ein Windows 64bit System benötigt

1) Installation OpenCV:
-Installationspfad: C:\opencv
-Umgebungsvariablen setzen
  - Unter Variable "Pfad" muss "C:\opencv\build\x64\vc14\bin" hinzufügt werden
  - Ggf. neue Systemvariable anlegen. Namens "OPENCV_DIR" mit Wert "C:\opencv\build\x64\vc14" hinzufügen
  
2) Installation IDS uEye SDK und IDS UI-1460SE-C-HG Treiber

3) Bedienung:
-Programm in Visual Studio 2015 starten
-Kalibrierungsstück unter die Kamera legen und "Calibrate" drücken
-Das zu testende Werkstück unter die Kamera legen und "Test" drücken
-Für weitere Überprüfungen auf "New Image" drücken
