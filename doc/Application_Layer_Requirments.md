# GUI Requirements  
Send and Download files
Open files in specific programs. Say downloads some files temp and render them in specific programs and download other files directly to device and save the file

(Sending Perspective)
## All Cases:
    file >> stream
(Receiving perspective)
## Case 1: Download files directly (Priority 1)
    stream >> file
## Case 2: Download file into temp and then push to a specific program
    stream >> temp && prog "temp"
## Case 3: Stream to another program
    stream | prog
