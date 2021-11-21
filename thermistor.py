import math
def therm_get_temp(v_out):    # Takes voltage on thermistor as input and returns its temperature
    
    #Constants
    BETA = 3435 #property of the thermistor
    T1 = 25
    R1 = 10000 # Known resistance of the thermistor at T1 temperature
    R_RES = 10000 #Resistance of the resistor in the circuit
    
    r_s = (v_out*R_RES)/(5-v_out) #Calculating a current resistance of the thermistor
    t2 = (T1 * BETA)/(BETA - (math.log(R1/r_s) * T1)) # using obtained resistance to find a tempurature at the thermistor
    return t2

