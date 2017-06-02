/* Модуль управления скважиной */

/* Переменные аппаратных входов */
const int StartButtonPin      = 9;     // Аппаратная кнопка запуска насоса скважины
const int impulsePin          = 2;     // Цифровой вход для подачи импульсов от счетчика для защиты насоса от сухого хода
const int TurbiditySensorPin  = A0;    // Аналоговый вход с датчика прозрачности

/* Переменные аппаратных выходов */
const int ledPin          =  13;       // Индикатор импульсов от счетчика
const int SkvajinaledPin  =  12;       // Выход включения насоса - подключается к автоматике скважины (HIGH-включено/LOW-выключено)
const int ledCleanWater   =  10;       // Выход на реле клапана чистой воды
const int ledDirtyWater   =  11;       // Выход на реле клапана грязной воды

/* Переменные значений входов и выходов */
int Turbidity   = 0;                   // Переменная для получения значений датчика прозрачности
int PulseState  = 0;                   // Переменная для получения значений входа импульсов


/* Служебные переменные и константы */
const long intervalDryWork    = 10000;   // Интервал за который проверяется количество импульсов счетчика (10 сек.) - защита насоса скважины от сухого хода
unsigned long previousMillis  = 0;       // Переменная для хранения интервала для подсчета импульсов
const int minTurbidity        = 350;     // Минимальное значение прозрачности (всё что меньше будет сливаться в дренаж)

int impulses            = 0;          // Переменная для подсчета импульсов внутри интервала (10 сек).
int CommulativeImpulses = 0;          // Накопительный счетчик импульсов - собирается за сеанс "откачки" (от старта до стопа!)
bool SkvajNasos         = false;      // Скважинный насос ВКЛ/ВЫКЛ

/* вход impulsePin может находиться в состоянии HIGH несколько циклов LOOP а нужно считать импульсом только одно его взведение в HIGH */
bool ImpulseUpdated = false;          // Переменная-флаг для корректного подсчета импульсов.


void setup() {

  Serial.begin(9600);
  // Инициализация входов/выходов
  pinMode(ledPin, OUTPUT);
  pinMode(SkvajinaledPin, OUTPUT);

  pinMode(ledCleanWater, OUTPUT);
  pinMode(ledDirtyWater, OUTPUT);

  pinMode(impulsePin, INPUT);
  pinMode(StartButtonPin, INPUT);
}

void loop()
{
  /*-------------------- BEGIN Блок работы скважины ----------------*/
  TurbidityCheck();                   // Проверка прозрачности
  StartSkvajinaRequest();             // Процедура старта насоса скважины - стартует по значению переменной SkvajNasos == true
  ScanImpulse();                      // Защита от сухого хода
  /*--------------------- END Блок работы скважины -----------------*/
}

/* Процедура проверки прозрачности воды
  Если насос работает, считываем показания датчика прозрачности.
  В случае если прозрачность меньше пороговой minTurbidity - вызываем процедуру "Грязной воды",
  иначе вызываем процедуру "Чистой воды".
*/
void TurbidityCheck()
{
  if (SkvajNasos == true)
  {
    Turbidity = analogRead(TurbiditySensorPin);
    if (Turbidity < minTurbidity)
    {
      DirtyWater();
    }
    else
    {
      CleanWater();
    }
    //Serial.println(Turbidity);
  }
  else
  {
    CloseWaterValves();
  }
}

/* Процедура защиты от сухого хода и непосредственного управления насосом скважины */
void ScanImpulse()
{
  unsigned long currentMillis = millis();

  PulseState = digitalRead(impulsePin);

  if (SkvajNasos == true)
  {
    TurnSkvajNasosOn();
  }
  else
  {
    TurnSkvajNasosOff();
  }

  if (currentMillis - previousMillis >= intervalDryWork)
  {
    previousMillis = currentMillis;
    if (SkvajNasos == true)
    {
      if (impulses < 10)
      {
        SkvajNasos = false;
        Serial.println("STOP...");
        Serial.println("TOTAL IMPULSES: ");
        Serial.println(CommulativeImpulses);
      }
      else
      {
        impulses = 0;
      }
    }
  }

  if (PulseState == HIGH)
  {
    if (ImpulseUpdated == false) {
      impulses++;
      CommulativeImpulses++;
      ImpulseUpdated = true;
      Serial.println(impulses);
    }
    digitalWrite(ledPin, HIGH);
  }
  else
  {
    ImpulseUpdated = false;
    digitalWrite(ledPin, LOW);
  }

}

/* Процедура включения скважинного насоса */
void StartSkvajinaRequest()
{
  int StartSkvajinaButton = 0;                          // Переменная для кнопки включения скважины
  StartSkvajinaButton = digitalRead(StartButtonPin);
  if (StartSkvajinaButton  == HIGH) {
    if (SkvajNasos == false)
    {
      CommulativeImpulses = 0;
      Serial.println("START NASOS");
      SkvajNasos = true;
    }
  }
}

/* Процедуры управления силовой частью - РЕЛЕ - BEGIN */
void TurnSkvajNasosOn(){
  digitalWrite(SkvajinaledPin, HIGH);
}

void TurnSkvajNasosOff(){
  digitalWrite(SkvajinaledPin, LOW);
}

void DirtyWater(){
  digitalWrite(ledDirtyWater, HIGH);
  digitalWrite(ledCleanWater, LOW);
}

void CleanWater(){
  digitalWrite(ledCleanWater, HIGH);
  digitalWrite(ledDirtyWater, LOW);
}

void CloseWaterValves(){
  digitalWrite(ledCleanWater, LOW);
  digitalWrite(ledDirtyWater, LOW);
}
/* Процедуры управления силовой частью - РЕЛЕ - END */
