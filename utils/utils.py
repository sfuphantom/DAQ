import re

""" 
Regex Matching
(?<=":) = Look for expression starting with ":
|(?<=,) = or Look for expression starting with ,
|(?<=\\[) = or Look for expression starting with \\[
-? = optional hyphen for negative number
\\d+ = number of any length
\\. = decimal point
\\d{x,} = number with x or more decimal points
"""
MIN_PRECISION = 3
float_match = re.compile(r'((?<=":)|(?<=,)|(?<=\[))-?\d+\.\d{'+str(MIN_PRECISION)+',}')

def mround(match, precision):
    """Round matched float to specified precision
    
    Parameters
    ----------
    match : Obj
        Matched expression
    precision : int
        Decimal places to round
    """   
    return ("{:."+str(precision)+"f}").format(float(match.group()))

def round_json(json_string, precision):
    """Round floats in JSON string to specified precision
    
    Parameters
    ----------
    json_string : string
        JSON string
    precision : int
        Decimal places to round
    """   
    return re.sub(float_match, lambda match :mround(match, precision), json_string)