/*
 * torqueTrajectoryTracking.h
 *
 *  Created on: Aug 7, 2018
 *      Author: Guillermo Herrera
 */

#ifndef BIOMECH_FLEXSEA_PROJECTS_MIT_DLEG_INC_TORQUETRAJECTORYTRACKING_H_
#define BIOMECH_FLEXSEA_PROJECTS_MIT_DLEG_INC_TORQUETRAJECTORYTRACKING_H_

#define TRAJ_SIZE 1000
#define USER_MASS 90.0
#define IMPEDANCE_MODE_THRESHOLD 100 // TODO: verify this

static const float torque_traj[TRAJ_SIZE] = {-1.8384,-1.6643,-1.5235,-1.4145,-1.3361,-1.2868,-1.2653,-1.27,-1.2997,-1.3529,-1.4283,-1.5243,-1.6398,-1.7732,-1.9231,-2.0882,-2.2671,-2.4583,-2.6605,-2.8723,-3.0922,-3.319,-3.5511,-3.7872,-4.026,-4.2659,-4.5056,-4.7438,-4.979,-5.2097,-5.4348,-5.6526,-5.8619,-6.0612,-6.2492,-6.4253,-6.5892,-6.7408,-6.8799,-7.0062,-7.1195,-7.2198,-7.3067,-7.3801,-7.4397,-7.4855,-7.5171,-7.5345,-7.5373,-7.5254,-7.4986,-7.4571,-7.4019,-7.3348,-7.2573,-7.1708,-7.077,-6.9774,-6.8736,-6.767,-6.6593,-6.552,-6.4467,-6.3448,-6.248,-6.1578,-6.0757,-6.0034,-5.9412,-5.888,-5.8426,-5.8038,-5.7703,-5.7408,-5.7141,-5.689,-5.6641,-5.6382,-5.61,-5.5784,-5.542,-5.4996,-5.45,-5.3918,-5.3239,-5.2463,-5.1595,-5.064,-4.9606,-4.8498,-4.7322,-4.6083,-4.4789,-4.3444,-4.2056,-4.0629,-3.9171,-3.7686,-3.6181,-3.4662,-3.3134,-3.1604,-3.0074,-2.8547,-2.7025,-2.551,-2.4006,-2.2514,-2.1038,-1.9578,-1.8139,-1.6722,-1.533,-1.3965,-1.263,-1.1327,-1.0059,-0.88276,-0.76339,-0.64743,-0.53456,-0.42443,-0.31671,-0.21104,-0.1071,-0.0045364,0.096986,0.19781,0.29827,0.39872,0.49948,0.60091,0.70334,0.80711,0.91255,1.0198,1.1287,1.2395,1.3519,1.466,1.5818,1.6993,1.8184,1.9391,2.0614,2.1852,2.3107,2.4376,2.566,2.6959,2.8273,2.9601,3.0942,3.2293,3.3654,3.5022,3.6396,3.7774,3.9155,4.0536,4.1916,4.3294,4.4667,4.6033,4.7392,4.8741,5.0079,5.1403,5.2714,5.4013,5.53,5.6576,5.7844,5.9104,6.0357,6.1605,6.2848,6.4088,6.5327,6.6565,6.7803,6.9043,7.0287,7.1534,7.2786,7.4044,7.5307,7.6574,7.7844,7.9118,8.0395,8.1674,8.2955,8.4237,8.5519,8.6802,8.8084,8.9365,9.0644,9.1922,9.3197,9.4469,9.5739,9.7006,9.8271,9.9535,10.08,10.206,10.332,10.458,10.584,10.71,10.837,10.963,11.09,11.217,11.344,11.471,11.599,11.727,11.855,11.982,12.11,12.238,12.365,12.492,12.619,12.744,12.87,12.994,13.118,13.24,13.362,13.482,13.601,13.719,13.836,13.952,14.067,14.182,14.296,14.409,14.522,14.635,14.748,14.861,14.974,15.088,15.202,15.316,15.432,15.548,15.665,15.782,15.9,16.019,16.137,16.256,16.375,16.494,16.613,16.732,16.85,16.968,17.085,17.201,17.317,17.432,17.546,17.658,17.77,17.881,17.992,18.101,18.209,18.317,18.424,18.53,18.635,18.74,18.844,18.947,19.05,19.152,19.253,19.354,19.454,19.553,19.653,19.752,19.85,19.948,20.046,20.143,20.24,20.337,20.434,20.53,20.627,20.723,20.82,20.916,21.012,21.108,21.204,21.3,21.395,21.49,21.585,21.679,21.772,21.865,21.957,22.049,22.14,22.23,22.318,22.406,22.493,22.58,22.665,22.749,22.833,22.915,22.997,23.079,23.16,23.24,23.32,23.4,23.479,23.557,23.636,23.714,23.792,23.87,23.948,24.025,24.103,24.179,24.256,24.332,24.407,24.482,24.556,24.63,24.703,24.775,24.846,24.917,24.986,25.055,25.123,25.19,25.257,25.322,25.387,25.452,25.515,25.579,25.641,25.704,25.766,25.827,25.888,25.949,26.01,26.071,26.131,26.192,26.252,26.312,26.372,26.432,26.492,26.552,26.611,26.67,26.73,26.789,26.847,26.906,26.964,27.023,27.081,27.139,27.196,27.254,27.311,27.369,27.426,27.483,27.541,27.598,27.655,27.712,27.769,27.827,27.884,27.942,27.999,28.057,28.115,28.173,28.231,28.289,28.348,28.406,28.465,28.524,28.583,28.642,28.702,28.761,28.821,28.881,28.941,29.001,29.062,29.123,29.183,29.244,29.305,29.366,29.428,29.489,29.55,29.612,29.673,29.735,29.796,29.858,29.919,29.981,30.042,30.104,30.165,30.227,30.289,30.351,30.413,30.476,30.54,30.604,30.669,30.734,30.801,30.868,30.936,31.005,31.075,31.147,31.219,31.292,31.367,31.442,31.518,31.595,31.672,31.749,31.828,31.906,31.985,32.063,32.142,32.221,32.3,32.378,32.456,32.534,32.612,32.69,32.768,32.846,32.925,33.004,33.083,33.162,33.242,33.323,33.405,33.487,33.57,33.654,33.739,33.825,33.912,34,34.089,34.179,34.269,34.361,34.453,34.546,34.639,34.734,34.829,34.924,35.02,35.117,35.214,35.312,35.41,35.509,35.608,35.708,35.808,35.909,36.01,36.112,36.215,36.318,36.422,36.527,36.632,36.738,36.844,36.952,37.06,37.168,37.278,37.388,37.5,37.612,37.725,37.839,37.954,38.069,38.186,38.304,38.424,38.544,38.665,38.788,38.912,39.037,39.163,39.29,39.419,39.548,39.678,39.81,39.942,40.075,40.209,40.343,40.478,40.614,40.751,40.888,41.025,41.163,41.302,41.44,41.58,41.72,41.861,42.002,42.144,42.287,42.431,42.575,42.72,42.866,43.013,43.16,43.309,43.459,43.609,43.761,43.913,44.067,44.221,44.376,44.532,44.689,44.847,45.005,45.164,45.324,45.485,45.646,45.808,45.971,46.135,46.299,46.464,46.629,46.795,46.962,47.13,47.298,47.468,47.638,47.809,47.981,48.154,48.328,48.502,48.678,48.855,49.032,49.211,49.39,49.571,49.752,49.934,50.117,50.301,50.485,50.669,50.855,51.041,51.227,51.413,51.6,51.788,51.975,52.163,52.351,52.54,52.729,52.918,53.107,53.297,53.488,53.678,53.87,54.062,54.254,54.447,54.64,54.834,55.029,55.225,55.421,55.618,55.815,56.013,56.211,56.41,56.61,56.81,57.01,57.211,57.412,57.614,57.816,58.018,58.22,58.423,58.626,58.829,59.033,59.236,59.44,59.644,59.848,60.053,60.257,60.462,60.667,60.873,61.078,61.284,61.49,61.696,61.903,62.109,62.316,62.523,62.73,62.937,63.144,63.351,63.557,63.764,63.97,64.176,64.382,64.587,64.792,64.996,65.2,65.403,65.606,65.808,66.01,66.211,66.411,66.611,66.81,67.008,67.206,67.403,67.599,67.795,67.99,68.184,68.378,68.57,68.762,68.954,69.144,69.333,69.522,69.71,69.896,70.082,70.266,70.449,70.631,70.812,70.992,71.17,71.347,71.523,71.697,71.869,72.041,72.21,72.378,72.545,72.71,72.874,73.037,73.198,73.357,73.515,73.672,73.827,73.981,74.133,74.284,74.434,74.582,74.728,74.873,75.016,75.158,75.298,75.435,75.571,75.705,75.837,75.966,76.093,76.218,76.341,76.461,76.579,76.694,76.806,76.916,77.023,77.128,77.23,77.329,77.426,77.52,77.612,77.701,77.788,77.872,77.953,78.032,78.109,78.183,78.254,78.322,78.388,78.452,78.512,78.57,78.625,78.677,78.726,78.772,78.815,78.855,78.892,78.926,78.957,78.984,79.009,79.03,79.047,79.062,79.073,79.08,79.085,79.086,79.083,79.077,79.068,79.055,79.039,79.019,78.996,78.969,78.939,78.905,78.867,78.825,78.78,78.73,78.676,78.618,78.555,78.487,78.414,78.336,78.253,78.165,78.071,77.972,77.866,77.755,77.638,77.515,77.387,77.254,77.116,76.973,76.826,76.674,76.519,76.36,76.198,76.033,75.866,75.695,75.523,75.348,75.172,74.994,74.813,74.631,74.446,74.259,74.069,73.875,73.679,73.478,73.274,73.067,72.854,72.638,72.417,72.191,71.959,71.723,71.481,71.233,70.979,70.719,70.452,70.178,69.896,69.607,69.311,69.006,68.693,68.371,68.04,67.7,67.351,66.991,66.622,66.243,65.854,65.456,65.048,64.63,64.203,63.767,63.321,62.866,62.402,61.93,61.448,60.957,60.458,59.95,59.433,58.908,58.375,57.835,57.287,56.732,56.17,55.602,55.027,54.447,53.862,53.271,52.676,52.076,51.472,50.864,50.253,49.638,49.02,48.399,47.775,47.148,46.517,45.883,45.246,44.605,43.961,43.314,42.664,42.01,41.353,40.693,40.029,39.362,38.692,38.019,37.342,36.663,35.982,35.298,34.612,33.924,33.234,32.543,31.85,31.156,30.462,29.766,29.07,28.373,27.676,26.979,26.283,25.587,24.892,24.198,23.506,22.815,22.127,21.442,20.759,20.079,19.402,18.729,18.06,17.396,16.736,16.081,15.431,14.788,14.151,13.522,12.902,12.29,11.688,11.096,10.516,9.9464,9.3896,8.8457,8.3155,7.7997,7.2988,6.8136,6.3445,5.8913,5.4538,5.0319,4.6254,4.2342,3.8581,3.4968,3.1503,2.8184,2.5008,2.1975,1.9083,1.633,1.3714,1.1233,0.8887,0.66729,0.45894,0.26348,0.080747,-0.089429,-0.24722,-0.39278,-0.5263,-0.64793,-0.75784,-0.85621,-0.94319,-1.019,-1.0837,-1.1375};

static const float massGains[TRAJ_SIZE] = {-0.036768,-0.033285,-0.030469,-0.028291,-0.026723,-0.025737,-0.025305,-0.0254,-0.025994,-0.027058,-0.028565,-0.030487,-0.032796,-0.035463,-0.038462,-0.041764,-0.045341,-0.049166,-0.05321,-0.057445,-0.061844,-0.066379,-0.071022,-0.075745,-0.080519,-0.085318,-0.090113,-0.094876,-0.099579,-0.10419,-0.1087,-0.11305,-0.11724,-0.12122,-0.12498,-0.12851,-0.13178,-0.13482,-0.1376,-0.14012,-0.14239,-0.1444,-0.14613,-0.1476,-0.14879,-0.14971,-0.15034,-0.15069,-0.15075,-0.15051,-0.14997,-0.14914,-0.14804,-0.1467,-0.14515,-0.14342,-0.14154,-0.13955,-0.13747,-0.13534,-0.13319,-0.13104,-0.12893,-0.1269,-0.12496,-0.12316,-0.12151,-0.12007,-0.11882,-0.11776,-0.11685,-0.11608,-0.11541,-0.11482,-0.11428,-0.11378,-0.11328,-0.11276,-0.1122,-0.11157,-0.11084,-0.10999,-0.109,-0.10784,-0.10648,-0.10493,-0.10319,-0.10128,-0.099212,-0.096996,-0.094643,-0.092166,-0.089578,-0.086889,-0.084112,-0.081259,-0.078341,-0.075371,-0.072361,-0.069323,-0.066268,-0.063208,-0.060148,-0.057094,-0.05405,-0.051021,-0.048012,-0.045029,-0.042075,-0.039157,-0.036278,-0.033444,-0.03066,-0.02793,-0.02526,-0.022654,-0.020117,-0.017655,-0.015268,-0.012949,-0.010691,-0.0084887,-0.0063341,-0.0042208,-0.002142,-9.0729e-05,0.0019397,0.0039562,0.0059654,0.0079743,0.0099896,0.012018,0.014067,0.016142,0.018251,0.020395,0.022575,0.024789,0.027038,0.029321,0.031637,0.033986,0.036368,0.038782,0.041228,0.043705,0.046213,0.048752,0.05132,0.053919,0.056546,0.059202,0.061883,0.064586,0.067308,0.070044,0.072793,0.075549,0.07831,0.081073,0.083833,0.086588,0.089333,0.092067,0.094784,0.097482,0.10016,0.10281,0.10543,0.10803,0.1106,0.11315,0.11569,0.11821,0.12071,0.12321,0.1257,0.12818,0.13065,0.13313,0.13561,0.13809,0.14057,0.14307,0.14557,0.14809,0.15061,0.15315,0.15569,0.15824,0.16079,0.16335,0.16591,0.16847,0.17104,0.1736,0.17617,0.17873,0.18129,0.18384,0.18639,0.18894,0.19148,0.19401,0.19654,0.19907,0.20159,0.20412,0.20664,0.20916,0.21168,0.21421,0.21673,0.21926,0.2218,0.22433,0.22687,0.22942,0.23197,0.23453,0.23709,0.23965,0.24221,0.24476,0.2473,0.24984,0.25237,0.25489,0.25739,0.25988,0.26235,0.26481,0.26724,0.26964,0.27203,0.27439,0.27672,0.27904,0.28134,0.28363,0.28591,0.28818,0.29044,0.2927,0.29496,0.29722,0.29948,0.30176,0.30404,0.30633,0.30863,0.31096,0.31329,0.31564,0.318,0.32037,0.32275,0.32513,0.32751,0.32988,0.33226,0.33463,0.337,0.33935,0.3417,0.34403,0.34634,0.34863,0.35091,0.35317,0.35541,0.35763,0.35983,0.36202,0.36419,0.36634,0.36848,0.3706,0.3727,0.3748,0.37688,0.37894,0.38099,0.38303,0.38506,0.38707,0.38908,0.39107,0.39305,0.39503,0.397,0.39896,0.40091,0.40286,0.4048,0.40674,0.40868,0.41061,0.41254,0.41446,0.41639,0.41832,0.42024,0.42216,0.42408,0.426,0.4279,0.4298,0.43169,0.43358,0.43545,0.4373,0.43915,0.44098,0.44279,0.44459,0.44637,0.44813,0.44987,0.45159,0.45329,0.45498,0.45665,0.45831,0.45995,0.46158,0.4632,0.4648,0.4664,0.46799,0.46957,0.47115,0.47272,0.47428,0.47585,0.47741,0.47896,0.48051,0.48205,0.48359,0.48511,0.48663,0.48814,0.48964,0.49112,0.49259,0.49405,0.49549,0.49692,0.49833,0.49973,0.5011,0.50246,0.50381,0.50513,0.50645,0.50775,0.50903,0.51031,0.51157,0.51283,0.51407,0.51531,0.51654,0.51777,0.51899,0.5202,0.52142,0.52263,0.52384,0.52504,0.52625,0.52745,0.52865,0.52984,0.53103,0.53222,0.53341,0.53459,0.53577,0.53695,0.53812,0.53929,0.54045,0.54161,0.54277,0.54393,0.54508,0.54623,0.54738,0.54852,0.54967,0.55081,0.55195,0.5531,0.55424,0.55539,0.55653,0.55768,0.55883,0.55998,0.56114,0.56229,0.56345,0.56462,0.56578,0.56695,0.56813,0.5693,0.57048,0.57166,0.57285,0.57403,0.57523,0.57642,0.57762,0.57882,0.58003,0.58124,0.58245,0.58367,0.58488,0.58611,0.58733,0.58855,0.58978,0.59101,0.59224,0.59347,0.5947,0.59593,0.59716,0.59839,0.59962,0.60085,0.60207,0.6033,0.60454,0.60578,0.60702,0.60827,0.60953,0.6108,0.61208,0.61338,0.61468,0.61601,0.61735,0.61872,0.6201,0.6215,0.62293,0.62438,0.62585,0.62734,0.62884,0.63036,0.63189,0.63344,0.63499,0.63655,0.63812,0.63969,0.64127,0.64284,0.64442,0.64599,0.64756,0.64912,0.65068,0.65224,0.6538,0.65536,0.65693,0.6585,0.66007,0.66165,0.66325,0.66485,0.66646,0.66809,0.66974,0.6714,0.67308,0.67478,0.6765,0.67824,0.68,0.68178,0.68357,0.68538,0.68721,0.68906,0.69091,0.69279,0.69467,0.69657,0.69848,0.70041,0.70234,0.70429,0.70624,0.7082,0.71018,0.71216,0.71416,0.71616,0.71818,0.72021,0.72225,0.7243,0.72637,0.72844,0.73054,0.73264,0.73476,0.73689,0.73903,0.74119,0.74337,0.74556,0.74777,0.74999,0.75223,0.75449,0.75677,0.75907,0.76139,0.76373,0.76609,0.76847,0.77088,0.77331,0.77576,0.77824,0.78074,0.78326,0.78581,0.78837,0.79096,0.79357,0.79619,0.79884,0.8015,0.80417,0.80686,0.80957,0.81228,0.81501,0.81775,0.8205,0.82326,0.82603,0.82881,0.8316,0.8344,0.83722,0.84004,0.84289,0.84574,0.84861,0.8515,0.8544,0.85732,0.86025,0.86321,0.86618,0.86917,0.87218,0.87521,0.87826,0.88133,0.88442,0.88752,0.89064,0.89378,0.89693,0.9001,0.90328,0.90648,0.9097,0.91293,0.91617,0.91942,0.92269,0.92598,0.92927,0.93258,0.93591,0.93924,0.9426,0.94597,0.94936,0.95276,0.95618,0.95962,0.96308,0.96655,0.97004,0.97356,0.97709,0.98064,0.98422,0.98781,0.99142,0.99504,0.99869,1.0023,1.006,1.0097,1.0134,1.0171,1.0208,1.0245,1.0283,1.032,1.0358,1.0395,1.0433,1.047,1.0508,1.0546,1.0584,1.0621,1.0659,1.0698,1.0736,1.0774,1.0812,1.0851,1.0889,1.0928,1.0967,1.1006,1.1045,1.1084,1.1124,1.1163,1.1203,1.1242,1.1282,1.1322,1.1362,1.1402,1.1442,1.1482,1.1523,1.1563,1.1604,1.1644,1.1685,1.1725,1.1766,1.1807,1.1847,1.1888,1.1929,1.197,1.2011,1.2051,1.2092,1.2133,1.2175,1.2216,1.2257,1.2298,1.2339,1.2381,1.2422,1.2463,1.2505,1.2546,1.2587,1.2629,1.267,1.2711,1.2753,1.2794,1.2835,1.2876,1.2917,1.2958,1.2999,1.304,1.3081,1.3121,1.3162,1.3202,1.3242,1.3282,1.3322,1.3362,1.3402,1.3441,1.3481,1.352,1.3559,1.3598,1.3637,1.3676,1.3714,1.3752,1.3791,1.3829,1.3867,1.3904,1.3942,1.3979,1.4016,1.4053,1.409,1.4126,1.4162,1.4198,1.4234,1.4269,1.4305,1.4339,1.4374,1.4408,1.4442,1.4476,1.4509,1.4542,1.4575,1.4607,1.464,1.4671,1.4703,1.4734,1.4765,1.4796,1.4827,1.4857,1.4887,1.4916,1.4946,1.4975,1.5003,1.5032,1.506,1.5087,1.5114,1.5141,1.5167,1.5193,1.5219,1.5244,1.5268,1.5292,1.5316,1.5339,1.5361,1.5383,1.5405,1.5426,1.5446,1.5466,1.5485,1.5504,1.5522,1.554,1.5558,1.5574,1.5591,1.5606,1.5622,1.5637,1.5651,1.5664,1.5678,1.569,1.5702,1.5714,1.5725,1.5735,1.5745,1.5754,1.5763,1.5771,1.5778,1.5785,1.5791,1.5797,1.5802,1.5806,1.5809,1.5812,1.5815,1.5816,1.5817,1.5817,1.5817,1.5815,1.5814,1.5811,1.5808,1.5804,1.5799,1.5794,1.5788,1.5781,1.5773,1.5765,1.5756,1.5746,1.5735,1.5724,1.5711,1.5697,1.5683,1.5667,1.5651,1.5633,1.5614,1.5594,1.5573,1.5551,1.5528,1.5503,1.5477,1.5451,1.5423,1.5395,1.5365,1.5335,1.5304,1.5272,1.524,1.5207,1.5173,1.5139,1.5105,1.507,1.5034,1.4999,1.4963,1.4926,1.4889,1.4852,1.4814,1.4775,1.4736,1.4696,1.4655,1.4613,1.4571,1.4528,1.4483,1.4438,1.4392,1.4345,1.4296,1.4247,1.4196,1.4144,1.409,1.4036,1.3979,1.3921,1.3862,1.3801,1.3739,1.3674,1.3608,1.354,1.347,1.3398,1.3324,1.3249,1.3171,1.3091,1.301,1.2926,1.2841,1.2753,1.2664,1.2573,1.248,1.2386,1.229,1.2191,1.2092,1.199,1.1887,1.1782,1.1675,1.1567,1.1457,1.1346,1.1234,1.112,1.1005,1.0889,1.0772,1.0654,1.0535,1.0415,1.0294,1.0173,1.0051,0.99277,0.98041,0.96799,0.9555,0.94295,0.93034,0.91766,0.90492,0.8921,0.87923,0.86629,0.85328,0.84021,0.82707,0.81386,0.80059,0.78725,0.77384,0.76037,0.74685,0.73327,0.71964,0.70596,0.69224,0.67848,0.66468,0.65086,0.637,0.62313,0.60923,0.59532,0.58139,0.56746,0.55352,0.53958,0.52565,0.51173,0.49783,0.48396,0.47011,0.45631,0.44254,0.42883,0.41517,0.40157,0.38804,0.37459,0.36121,0.34791,0.33471,0.32161,0.30862,0.29575,0.28303,0.27045,0.25804,0.2458,0.23376,0.22193,0.21031,0.19893,0.18779,0.17691,0.16631,0.15599,0.14598,0.13627,0.12689,0.11783,0.10908,0.10064,0.092509,0.084684,0.077161,0.069936,0.063006,0.056367,0.050017,0.043951,0.038166,0.032659,0.027427,0.022467,0.017774,0.013346,0.0091787,0.0052696,0.0016149,-0.0017886,-0.0049443,-0.0078557,-0.010526,-0.012959,-0.015157,-0.017124,-0.018864,-0.020379,-0.021674,-0.022751};

static const float speedGains[TRAJ_SIZE] = {-0.39102,-0.37312,-0.35802,-0.34568,-0.336,-0.32892,-0.32438,-0.32234,-0.3227,-0.32544,-0.33046,-0.33774,-0.34716,-0.35872,-0.37232,-0.38788,-0.36714,-0.3843,-0.40352,-0.42478,-0.44798,-0.47308,-0.5,-0.52866,-0.55904,-0.59106,-0.62464,-0.65974,-0.69632,-0.73424,-0.77352,-0.81404,-0.85576,-0.89856,-0.94234,-0.98656,-1.0308,-1.0744,-1.1172,-1.1583,-1.1972,-1.2339,-1.2674,-1.2973,-1.3231,-1.3444,-1.3605,-1.3711,-1.3809,-1.3818,-1.3754,-1.3608,-1.3389,-1.31,-1.2752,-1.2349,-1.1901,-1.1414,-1.0895,-1.0354,-0.97946,-0.92264,-0.8654,-0.80884,-0.7535,-0.7001,-0.64962,-0.60334,-0.56196,-0.5254,-0.49354,-0.4662,-0.44322,-0.42446,-0.4098,-0.39904,-0.39206,-0.38868,-0.38876,-0.39214,-0.4103,-0.42178,-0.43608,-0.45272,-0.4712,-0.49102,-0.51182,-0.5332,-0.55478,-0.57616,-0.59696,-0.61676,-0.6352,-0.65186,-0.66638,-0.67836,-0.6874,-0.69312,-0.6951,-0.69306,-0.68666,-0.67568,-0.66042,-0.64124,-0.61852,-0.59266,-0.56408,-0.5331,-0.50014,-0.46558,-0.42982,-0.39322,-0.35618,-0.34371,-0.3062,-0.26951,-0.23415,-0.20061,-0.16922,-0.13996,-0.11282,-0.087777,-0.064821,-0.043932,-0.025088,-0.008274,0.006532,0.019344,0.030186,0.039078,0.04603,0.0511,0.05442,0.05604,0.05606,0.05462,0.05184,0.04786,0.04282,0.03686,0.03012,0.02276,0.01484,0.0066,-0.00186,-0.0367,-0.0455,-0.05408,-0.06224,-0.06988,-0.07676,-0.08278,-0.08788,-0.0921,-0.0954,-0.0978,-0.09928,-0.09984,-0.09948,-0.09818,-0.09596,-0.09282,-0.08872,-0.08372,-0.07786,-0.0712,-0.06374,-0.05562,-0.04686,-0.03758,-0.02786,-0.0178,-0.00752,0.00286,0.0133,0.02366,0.03384,0.01878,0.02822,0.03716,0.04554,0.05334,0.06046,0.06684,0.0724,0.07718,0.08122,0.08458,0.08732,0.0895,0.09116,0.09238,0.09322,0.0937,0.0939,0.09388,0.09362,0.09328,0.09278,0.0923,0.0916,0.0912,0.0906,0.0902,0.0896,0.0892,0.0886,0.0884,0.088,0.0876,0.062,0.0618,0.0616,0.0616,0.062,0.0622,0.0628,0.0636,0.0648,0.066,0.0678,0.0696,0.0718,0.074,0.0764,0.079,0.082,0.0848,0.088,0.091,0.094,0.097,0.0998,0.1024,0.105,0.1072,0.109,0.1106,0.1118,0.1126,0.1132,0.1134,0.09,0.0894,0.0884,0.087,0.0854,0.0832,0.0812,0.0788,0.0762,0.0736,0.0708,0.0678,0.0648,0.0618,0.059,0.056,0.0532,0.0502,0.0474,0.0448,0.042,0.0394,0.0368,0.0342,0.0318,0.0292,0.0268,0.0244,0.0222,0.0196,0.0172,0.015,-0.0078,-0.01,-0.0124,-0.0146,-0.0168,-0.0192,-0.0218,-0.024,-0.0266,-0.0292,-0.0318,-0.0346,-0.0376,-0.0406,-0.0434,-0.0466,-0.0498,-0.0534,-0.0566,-0.0602,-0.0638,-0.0674,-0.0712,-0.0748,-0.0784,-0.0822,-0.0858,-0.0892,-0.0928,-0.0962,-0.0998,-0.1032,-0.1064,-0.1268,-0.1298,-0.1326,-0.1352,-0.1378,-0.1404,-0.1428,-0.1452,-0.1478,-0.1502,-0.1526,-0.1552,-0.158,-0.1606,-0.1632,-0.166,-0.169,-0.172,-0.1752,-0.1786,-0.1818,-0.1856,-0.189,-0.1928,-0.1966,-0.2004,-0.2044,-0.2082,-0.2124,-0.2166,-0.2206,-0.2246,-0.2426,-0.2466,-0.2504,-0.2544,-0.2582,-0.262,-0.2656,-0.2696,-0.2732,-0.277,-0.2806,-0.2844,-0.288,-0.2916,-0.2952,-0.299,-0.3028,-0.3066,-0.3102,-0.3142,-0.318,-0.3218,-0.3258,-0.3298,-0.334,-0.338,-0.342,-0.3462,-0.3504,-0.3546,-0.3586,-0.363,-0.3788,-0.383,-0.3872,-0.3912,-0.3956,-0.3996,-0.4038,-0.4078,-0.412,-0.4162,-0.4202,-0.4242,-0.4282,-0.4322,-0.4364,-0.4402,-0.4444,-0.4482,-0.4522,-0.4564,-0.4604,-0.4646,-0.4688,-0.473,-0.4774,-0.4818,-0.4864,-0.4908,-0.4956,-0.5004,-0.5052,-0.5104,-0.5156,-0.5326,-0.538,-0.5436,-0.5488,-0.5542,-0.5596,-0.5648,-0.5704,-0.5756,-0.5808,-0.586,-0.591,-0.596,-0.6006,-0.6054,-0.6098,-0.6142,-0.6184,-0.6226,-0.6266,-0.6304,-0.6342,-0.638,-0.6416,-0.6452,-0.649,-0.6528,-0.6566,-0.6602,-0.6642,-0.668,-0.672,-0.6902,-0.6948,-0.6992,-0.7038,-0.7088,-0.7136,-0.7184,-0.7234,-0.7282,-0.733,-0.738,-0.7426,-0.7474,-0.7518,-0.7562,-0.7606,-0.7648,-0.7686,-0.7724,-0.7758,-0.7792,-0.7822,-0.7852,-0.7878,-0.7904,-0.7928,-0.795,-0.7968,-0.7986,-0.8002,-0.8018,-0.803,-0.821,-0.8222,-0.8232,-0.8242,-0.8252,-0.826,-0.8266,-0.827,-0.8276,-0.828,-0.8282,-0.8282,-0.8284,-0.8284,-0.828,-0.8278,-0.8274,-0.8268,-0.8264,-0.8256,-0.8248,-0.8238,-0.8228,-0.8216,-0.8204,-0.819,-0.8176,-0.8162,-0.8146,-0.813,-0.8114,-0.8096,-0.8076,-0.8272,-0.8254,-0.8234,-0.8214,-0.8194,-0.8174,-0.8152,-0.813,-0.8108,-0.8084,-0.8058,-0.8034,-0.8008,-0.7984,-0.7958,-0.7928,-0.7902,-0.7874,-0.7844,-0.7814,-0.7782,-0.775,-0.7716,-0.768,-0.7644,-0.7604,-0.7564,-0.752,-0.7474,-0.7426,-0.7376,-0.7324,-0.7542,-0.7484,-0.7424,-0.7356,-0.729,-0.722,-0.7148,-0.7072,-0.6994,-0.6914,-0.6832,-0.6748,-0.666,-0.6572,-0.6482,-0.6388,-0.6294,-0.62,-0.6102,-0.6004,-0.5902,-0.5802,-0.5698,-0.5592,-0.5486,-0.5378,-0.5268,-0.5154,-0.5038,-0.4922,-0.4804,-0.4682,-0.4884,-0.4762,-0.4636,-0.4508,-0.4376,-0.4242,-0.411,-0.3974,-0.3836,-0.37,-0.356,-0.342,-0.328,-0.3138,-0.2998,-0.2854,-0.2714,-0.2572,-0.243,-0.229,-0.2148,-0.2008,-0.1866,-0.1726,-0.1584,-0.1442,-0.1298,-0.1154,-0.101,-0.0862,-0.0714,-0.0562,-0.041,-0.063,-0.0474,-0.0314,-0.0154,0.0008,0.0174,0.0342,0.0512,0.068,0.0854,0.1024,0.1196,0.1368,0.1542,0.1714,0.1888,0.2058,0.2228,0.2396,0.2564,0.2732,0.2896,0.3062,0.3226,0.3388,0.3552,0.3716,0.3878,0.4042,0.4204,0.4368,0.4532,0.4294,0.446,0.4628,0.4796,0.4966,0.5138,0.5312,0.5488,0.5662,0.584,0.6016,0.6194,0.6372,0.655,0.6728,0.6906,0.7084,0.726,0.7438,0.7614,0.779,0.7964,0.8138,0.8312,0.8484,0.8658,0.8828,0.9,0.9172,0.9342,0.9514,0.9684,0.9448,0.962,0.9792,0.9966,1.0138,1.031,1.0484,1.0656,1.083,1.1004,1.1178,1.135,1.1524,1.1698,1.187,1.2044,1.2216,1.239,1.2562,1.2732,1.2904,1.3076,1.3244,1.3414,1.3586,1.3754,1.3924,1.4092,1.4258,1.4424,1.4588,1.4752,1.4914,1.4726,1.489,1.505,1.521,1.5366,1.552,1.5674,1.5822,1.5968,1.6112,1.6256,1.6394,1.6528,1.6662,1.6792,1.6918,1.7042,1.7162,1.7278,1.7392,1.7504,1.7612,1.7714,1.7816,1.7916,1.8012,1.8106,1.8196,1.8286,1.8372,1.8458,1.8538,1.8382,1.8466,1.8548,1.8628,1.8706,1.878,1.8854,1.8926,1.8996,1.9062,1.9128,1.919,1.925,1.9308,1.9364,1.9418,1.9466,1.9514,1.9562,1.9606,1.9648,1.9684,1.9722,1.9756,1.9788,1.982,1.9848,1.9872,1.9896,1.9914,1.993,1.9942,1.9896,1.9906,1.9914,1.9918,1.9916,1.991,1.99,1.9882,1.9858,1.9832,1.98,1.976,1.9716,1.9668,1.9614,1.9556,1.9494,1.9426,1.9356,1.9282,1.9204,1.9122,1.904,1.8956,1.8868,1.878,1.8692,1.8604,1.8514,1.8424,1.8332,1.824,1.8148,1.828,1.82,1.8118,1.8036,1.795,1.7862,1.7772,1.7676,1.7576,1.7468,1.7356,1.7234,1.7102,1.6958,1.6806,1.6638,1.646,1.6264,1.6052,1.5826,1.5582,1.5324,1.5046,1.4752,1.4444,1.4116,1.3774,1.3416,1.3044,1.2666,1.2276,1.1882,1.1946,1.1556,1.1166,1.078,1.0398,1.0024,0.9658,0.9304,0.8966,0.8644,0.8338,0.8052,0.7784,0.7534,0.7304,0.709,0.6894,0.6716,0.6556,0.641,0.628,0.6162,0.6058,0.5968,0.5888,0.5816,0.5756,0.5704,0.5658,0.5616,0.558,0.5546,0.6528,0.6514,0.6496,0.6476,0.6448,0.6414,0.637,0.6316,0.6248,0.6166,0.6066,0.5948,0.5812,0.5654,0.548,0.5286,0.5076,0.4846,0.4602,0.4342,0.4066,0.3776,0.3476,0.3164,0.2842,0.251,0.2172,0.1826,0.1476,0.112,0.0762,0.04,0.0038,0.101,0.0656,0.0304,-0.0044,-0.039,-0.0732,-0.1068,-0.14,-0.1724,-0.2042,-0.2352,-0.2652,-0.2944,-0.3228,-0.3498,-0.376,-0.4008,-0.4248,-0.4474,-0.4688,-0.489,-0.5078,-0.5254,-0.5418,-0.5566,-0.5702,-0.5826,-0.5934,-0.6028,-0.6106,-0.617,-0.6222,-0.494,-0.4974,-0.4996,-0.5008,-0.5008,-0.5002,-0.4988,-0.49654,-0.4938,-0.49052,-0.48708,-0.4831,-0.47906,-0.4749,-0.47074,-0.46664,-0.46264,-0.4588,-0.45514,-0.45152,-0.44784,-0.444,-0.4399,-0.43542,-0.43044,-0.42484,-0.41852,-0.4114,-0.40331,-0.39419,-0.38395,-0.37272,-0.31101,-0.30092,-0.29024,-0.27915,-0.2678,-0.25635,-0.24493,-0.23374,-0.22288,-0.21256,-0.20289,-0.19405,-0.1862,-0.17946,-0.17402,-0.16998,-0.16734,-0.17472,-0.18138,-0.18738,-0.19272,-0.19748,-0.20166,-0.2053,-0.20844,-0.21114,-0.2134,-0.21526,-0.21676,-0.21796,-0.21886,-0.2195,-0.21994,-0.2202,-0.2203,-0.2203,-0.22022,-0.2201,-0.21998,-0.21988,-0.21986,-0.21994,-0.22016,-0.22054,-0.22112,-0.22196,-0.22306};

#endif /* BIOMECH_FLEXSEA_PROJECTS_MIT_DLEG_INC_TORQUETRAJECTORYTRACKING_H_ */
