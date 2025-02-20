void analogSensors(){

  digitalWrite(27, LOW); // y on
  digitalWrite(28, HIGH); //g off
  
  allSensors[4] = (convertSensor(analogRead(FL_BRK_TMP_PIN)) - 0.50)/0.005;
  allSensors[5] = (convertSensor(analogRead(FR_BRK_TMP_PIN)) - 0.50)/0.005;
  allSensors[6] = can_read(4); // back l brake temp
  allSensors[7] = can_read(5); // back r brake temp
  allSensors[8] = convertSensor(analogRead(FL_SUS_POT_PIN), FL_SUS_POT_CLB);
  allSensors[9] = convertSensor(analogRead(FR_SUS_POT_PIN), FR_SUS_POT_CLB);
  //  allSensors[10] = back l suspot
  //  allSensors[11] = back r suspot
  allSensors[12] = (convertSensor(analogRead(F_BRK_PRES_PIN), F_BRK_PRES_CLB) - 0.50)/ (4/7500);
  allSensors[13] = can_read(1); // back brk pres
  allSensors[14] = convertSensor(analogRead(STEER_ANG_PIN), STEER_ANG_CLB);
  // motec
  allSensors[15] = can_read(31); // TPS
  allSensors[16] = can_read(20); // OIL_PRES
  allSensors[17] = can_read(21); // OIL_TEMP
  allSensors[18] = can_read(53); // COOL_TEMP
  allSensors[19] = can_read(51); // MAP
  allSensors[20] = can_read(52); // MAT
  allSensors[21] = can_read(30); // NEUT
  allSensors[22] = can_read(40); // LAMBDA1
  allSensors[23] = can_read(41); // LAMBDA2

  // i2c
  //  allSensors[24] = accel;
  //  allSensors[25] = gyro;
  //  allSensors[26] = gps;

  // strain gauges
  allSensors[27] = convertSensor(ads1115b.readADC_Differential_0_1(), STRAIN1_CLB);
  allSensors[28] = convertSensor(ads1115b.readADC_Differential_2_3(), STRAIN2_CLB);
  allSensors[29] = convertSensor(ads1115c.readADC_Differential_0_1(), STRAIN3_CLB);
  allSensors[30] = convertSensor(ads1115c.readADC_Differential_2_3(), STRAIN4_CLB);

  allSensors[31] = convertSensor(analogRead(PTUBE1_PIN), PTUBE_CLB);
  allSensors[32] = convertSensor(analogRead(PTUBE2_PIN), PTUBE_CLB);
  allSensors[33] = convertSensor(analogRead(PTUBE3_PIN), PTUBE_CLB);
  allSensors[34] = convertSensor(analogRead(PTUBE4_PIN), PTUBE_CLB);
  allSensors[35] = convertSensor(analogRead(PTUBE5_PIN), PTUBE_CLB);
  allSensors[36] = convertSensor(analogRead(PTUBE6_PIN), PTUBE_CLB);
  allSensors[37] = convertSensor(analogRead(PTUBE7_PIN), PTUBE_CLB);
  allSensors[38] = convertSensor(analogRead(PTUBE8_PIN), PTUBE_CLB);
  allSensors[39] = convertSensor(analogRead(PTUBE9_PIN), PTUBE_CLB);
  allSensors[40] = convertSensor(analogRead(PTUBE10_PIN), PTUBE_CLB);
  allSensors[41] = convertSensor(ads1115a.readADC_SingleEnded(0));
  allSensors[42] = convertSensor(ads1115a.readADC_SingleEnded(0));
}
