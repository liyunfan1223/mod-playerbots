DELETE FROM ai_playerbot_texts WHERE name IN (
    'gameobject_unavailable_error',
    'gameobject_disallowed_error',
    'gameobject_outside_loot_distance_error',
    'gameobject_not_eligible_error',
    'gameobject_cannot_interact_error',
    'gameobject_cannot_loot_error',
    'gameobject_stay_out_of_range_error',
    'gameobject_approach_unavailable_error',
    'gameobject_open_failed_error',
    'gameobject_open_requested',
    'gameobject_loot_queued',
    'gameobject_nonloot_locked_error',
    'gameobject_interact_out_of_range_error',
    'gameobject_interaction_requested'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'gameobject_unavailable_error',
    'gameobject_disallowed_error',
    'gameobject_outside_loot_distance_error',
    'gameobject_not_eligible_error',
    'gameobject_cannot_interact_error',
    'gameobject_cannot_loot_error',
    'gameobject_stay_out_of_range_error',
    'gameobject_approach_unavailable_error',
    'gameobject_open_failed_error',
    'gameobject_open_requested',
    'gameobject_loot_queued',
    'gameobject_nonloot_locked_error',
    'gameobject_interact_out_of_range_error',
    'gameobject_interaction_requested'
);

INSERT INTO ai_playerbot_texts (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8) VALUES
(1914, 'gameobject_unavailable_error', 'Game object is no longer available', 0, 0, '게임 오브젝트를 더 이상 사용할 수 없습니다', 'L''objet de jeu n''est plus disponible', 'Das Spielobjekt ist nicht mehr verfügbar', '游戏物体不再可用', '遊戲物件不再可用', 'El objeto de juego ya no está disponible', 'El objeto de juego ya no está disponible', 'Игровой объект больше недоступен'),
(1915, 'gameobject_disallowed_error', 'Game object is disallowed by configuration', 0, 0, '설정에서 이 게임 오브젝트를 허용하지 않습니다', 'L''objet de jeu est interdit par la configuration', 'Das Spielobjekt ist durch die Konfiguration gesperrt', '配置不允许该游戏物体', '設定不允許此遊戲物件', 'La configuración no permite este objeto de juego', 'La configuración no permite este objeto de juego', 'Игровой объект запрещён настройками'),
(1916, 'gameobject_outside_loot_distance_error', 'Game object is outside the configured loot distance', 0, 0, '게임 오브젝트가 설정된 전리품 획득 거리 밖에 있습니다', 'L''objet de jeu est hors de la distance de butin configurée', 'Das Spielobjekt befindet sich außerhalb der konfigurierten Plünderdistanz', '游戏物体超出配置的拾取距离', '遊戲物件超出設定的拾取距離', 'El objeto de juego está fuera de la distancia de botín configurada', 'El objeto de juego está fuera de la distancia de botín configurada', 'Игровой объект находится вне настроенной дистанции добычи'),
(1917, 'gameobject_not_eligible_error', 'Game object is not currently eligible for interaction', 0, 0, '현재 이 게임 오브젝트와 상호작용할 수 없습니다', 'L''objet de jeu ne peut pas être utilisé actuellement', 'Mit diesem Spielobjekt kann derzeit nicht interagiert werden', '当前无法与该游戏物体交互', '目前無法與此遊戲物件互動', 'No se puede interactuar con este objeto de juego actualmente', 'No se puede interactuar con este objeto de juego actualmente', 'С этим игровым объектом сейчас нельзя взаимодействовать'),
(1918, 'gameobject_cannot_interact_error', 'Cannot interact while dead, flying, remotely controlled, casting, or looting', 0, 0, '죽음, 비행, 원격 조종, 시전 또는 전리품 획득 중에는 상호작용할 수 없습니다', 'Impossible d''interagir en étant mort, en vol, contrôlé à distance, en incantation ou en train de piller', 'Interaktion nicht möglich, während ich tot bin, fliege, ferngesteuert werde, wirke oder plündere', '死亡、飞行、被远程控制、施法或拾取时无法交互', '死亡、飛行、被遠端控制、施法或拾取時無法互動', 'No puedo interactuar mientras estoy muerto, volando, controlado a distancia, lanzando un hechizo o saqueando', 'No puedo interactuar mientras estoy muerto, volando, controlado a distancia, lanzando un hechizo o saqueando', 'Нельзя взаимодействовать, будучи мёртвым, в полёте, под дистанционным управлением, во время чтения заклинания или добычи'),
(1919, 'gameobject_cannot_loot_error', 'Cannot loot this object: check quest, skill, tools, key, and object state', 0, 0, '이 오브젝트에서 전리품을 얻을 수 없습니다: 퀘스트, 기술, 도구, 열쇠 및 오브젝트 상태를 확인하세요', 'Impossible de piller cet objet : vérifiez la quête, la compétence, les outils, la clé et l''état de l''objet', 'Dieses Objekt kann nicht geplündert werden: Quest, Fertigkeit, Werkzeuge, Schlüssel und Objektzustand prüfen', '无法拾取该物体：请检查任务、技能、工具、钥匙和物体状态', '無法拾取此物件：請檢查任務、技能、工具、鑰匙和物件狀態', 'No puedo saquear este objeto: revisa la misión, habilidad, herramientas, llave y estado del objeto', 'No puedo saquear este objeto: revisa la misión, habilidad, herramientas, llave y estado del objeto', 'Нельзя обыскать этот объект: проверьте задание, навык, инструменты, ключ и состояние объекта'),
(1920, 'gameobject_stay_out_of_range_error', 'Game object is out of reach while staying', 0, 0, '제자리에 머무는 동안 게임 오브젝트에 닿을 수 없습니다', 'L''objet de jeu est hors de portée tant que je reste sur place', 'Das Spielobjekt ist außer Reichweite, während ich stehen bleibe', '停留时游戏物体无法触及', '停留時遊戲物件無法觸及', 'El objeto de juego está fuera de alcance mientras permanezco aquí', 'El objeto de juego está fuera de alcance mientras permanezco aquí', 'Игровой объект вне досягаемости, пока я остаюсь на месте'),
(1921, 'gameobject_approach_unavailable_error', 'Move closer or enable the loot or gather strategy to approach this object', 0, 0, '더 가까이 이동하거나 전리품 또는 채집 전략을 활성화하여 이 오브젝트에 접근하세요', 'Approchez-vous ou activez la stratégie de butin ou de récolte pour atteindre cet objet', 'Gehe näher heran oder aktiviere die Plünder- oder Sammelstrategie, um dieses Objekt zu erreichen', '请靠近，或启用拾取或采集策略以接近该物体', '請靠近，或啟用拾取或採集策略以接近此物件', 'Acércate o activa la estrategia de botín o recolección para aproximarte a este objeto', 'Acércate o activa la estrategia de botín o recolección para aproximarte a este objeto', 'Подойдите ближе или включите стратегию добычи либо сбора, чтобы подойти к этому объекту'),
(1922, 'gameobject_open_failed_error', 'Could not approach or open the game object', 0, 0, '게임 오브젝트에 접근하거나 열 수 없습니다', 'Impossible d''approcher ou d''ouvrir l''objet de jeu', 'Das Spielobjekt konnte nicht erreicht oder geöffnet werden', '无法靠近或打开游戏物体', '無法靠近或開啟遊戲物件', 'No pude acercarme ni abrir el objeto de juego', 'No pude acercarme ni abrir el objeto de juego', 'Не удалось подойти к игровому объекту или открыть его'),
(1923, 'gameobject_open_requested', 'Opening requested: %gameobject', 0, 0, '열기 요청됨: %gameobject', 'Ouverture demandée : %gameobject', 'Öffnen angefordert: %gameobject', '已请求打开：%gameobject', '已請求開啟：%gameobject', 'Apertura solicitada: %gameobject', 'Apertura solicitada: %gameobject', 'Запрошено открытие: %gameobject'),
(1924, 'gameobject_loot_queued', 'Queued for looting: %gameobject', 0, 0, '전리품 획득 대기열에 추가됨: %gameobject', 'Mis en file d''attente pour le butin : %gameobject', 'Zum Plündern vorgemerkt: %gameobject', '已加入拾取队列：%gameobject', '已加入拾取佇列：%gameobject', 'En cola para saquear: %gameobject', 'En cola para saquear: %gameobject', 'Добавлено в очередь на добычу: %gameobject'),
(1925, 'gameobject_nonloot_locked_error', 'This non-loot object requires an opening spell or key', 0, 0, '이 비전리품 오브젝트에는 열기 주문이나 열쇠가 필요합니다', 'Cet objet sans butin nécessite un sort d''ouverture ou une clé', 'Dieses Nicht-Beuteobjekt benötigt einen Öffnungszauber oder Schlüssel', '此非拾取物体需要开启法术或钥匙', '此非拾取物件需要開啟法術或鑰匙', 'Este objeto sin botín requiere un hechizo de apertura o una llave', 'Este objeto sin botín requiere un hechizo de apertura o una llave', 'Для этого объекта без добычи требуется заклинание открытия или ключ'),
(1926, 'gameobject_interact_out_of_range_error', 'Move closer to interact with this game object', 0, 0, '이 게임 오브젝트와 상호작용하려면 더 가까이 이동하세요', 'Approchez-vous pour interagir avec cet objet de jeu', 'Gehe näher heran, um mit diesem Spielobjekt zu interagieren', '请靠近以与该游戏物体交互', '請靠近以與此遊戲物件互動', 'Acércate para interactuar con este objeto de juego', 'Acércate para interactuar con este objeto de juego', 'Подойдите ближе, чтобы взаимодействовать с этим игровым объектом'),
(1927, 'gameobject_interaction_requested', 'Game object interaction requested', 0, 0, '게임 오브젝트 상호작용이 요청되었습니다', 'Interaction avec l''objet de jeu demandée', 'Interaktion mit dem Spielobjekt angefordert', '已请求与游戏物体交互', '已請求與遊戲物件互動', 'Interacción con el objeto de juego solicitada', 'Interacción con el objeto de juego solicitada', 'Запрошено взаимодействие с игровым объектом');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
('gameobject_unavailable_error', 100),
('gameobject_disallowed_error', 100),
('gameobject_outside_loot_distance_error', 100),
('gameobject_not_eligible_error', 100),
('gameobject_cannot_interact_error', 100),
('gameobject_cannot_loot_error', 100),
('gameobject_stay_out_of_range_error', 100),
('gameobject_approach_unavailable_error', 100),
('gameobject_open_failed_error', 100),
('gameobject_open_requested', 100),
('gameobject_loot_queued', 100),
('gameobject_nonloot_locked_error', 100),
('gameobject_interact_out_of_range_error', 100),
('gameobject_interaction_requested', 100);
