import logging

def iter(i):
    list = []
    for y in i:
        list.append(y)
    logging.info(list)
    return list
    
iter(range(3))
iter(range(2, 3))
iter(range(10, 0, -1))

logging.info('range=', range)
logging.info('range(5)=', range(5))