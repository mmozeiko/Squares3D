FasdUAS 1.101.10   ��   ��    k             i         I     �� 	��
�� .aevtodocnull  �    alis 	 l      
�� 
 o      ���� 0 droppeditems DroppedItems��  ��    Y     � ��  ��  k    �       r        n       4    �� 
�� 
cobj  1    ��
�� 
pidx  o    ���� 0 droppeditems DroppedItems  o      ���� (0 pathtosourcefolder PathToSourceFolder      r        l    ��  I   �� ��
�� .sysonfo4asfe       fss   o    ���� (0 pathtosourcefolder PathToSourceFolder��  ��    o      ����  0 sourceiteminfo sourceItemInfo      r    $    l   "  ��   n    " ! " ! 1     "��
�� 
kind " o     ����  0 sourceiteminfo sourceItemInfo��    o      ���� &0 kindofdroppeditem kindOfDroppedItem   # $ # l  % %������  ��   $  %�� % Z   % � & '�� ( & G   % 0 ) * ) l  % ( +�� + =  % ( , - , o   % &���� &0 kindofdroppeditem kindOfDroppedItem - m   & ' . .  Folder   ��   * l  + . /�� / =  + . 0 1 0 o   + ,���� &0 kindofdroppeditem kindOfDroppedItem 1 m   , - 2 2  Volume   ��   ' k   3 � 3 3  4 5 4 I  3 P�� 6 7
�� .sysodlogaskr        TEXT 6 b   3 > 8 9 8 b   3 < : ; : b   3 : < = < b   3 8 > ? > b   3 6 @ A @ m   3 4 B B 1 +Creating Compressed Disk Image from folder:    A o   4 5��
�� 
ret  ? o   6 7��
�� 
ret  = m   8 9 C C  '    ; o   : ;���� (0 pathtosourcefolder PathToSourceFolder 9 m   < = D D  '    7 �� E F
�� 
btns E J   ? D G G  H I H m   ? @ J J  Cancel    I  K L K m   @ A M M  Don't create    L  N�� N m   A B O O  Create Image   ��   F �� P Q
�� 
dflt P m   E F����  Q �� R��
�� 
disp R m   I J���� ��   5  S T S r   Q Z U V U l  Q X W�� W n   Q X X Y X 1   T X��
�� 
bhit Y l  Q T Z�� Z 1   Q T��
�� 
rslt��  ��   V l      [�� [ o      ���� 0 
userchoice 
userChoice��   T  \ ] \ l  [ [������  ��   ]  ^�� ^ Z   [ � _ `���� _ =  [ ` a b a o   [ \���� 0 
userchoice 
userChoice b m   \ _ c c  Create Image    ` k   c � d d  e f e r   c j g h g l  c h i�� i n   c h j k j 1   d h��
�� 
dnam k o   c d����  0 sourceiteminfo sourceItemInfo��   h o      ���� 00 nameofmounteddiscimage NameOfMountedDiscImage f  l m l r   k r n o n n   k p p q p 1   l p��
�� 
psxp q l  k l r�� r o   k l���� (0 pathtosourcefolder PathToSourceFolder��   o o      ���� (0 pathtosourcefolder PathToSourceFolder m  s t s r   s � u v u b   s ~ w x w b   s z y z y b   s x { | { o   s t���� (0 pathtosourcefolder PathToSourceFolder | m   t w } } 	 ../    z o   x y���� 00 nameofmounteddiscimage NameOfMountedDiscImage x m   z } ~ ~ 
 .dmg    v o      ���� ,0 pathtocreateddmgfile PathToCreatedDMGFile t   �  l  � �������  ��   �  � � � l  � ��� ���   � � �				set myShellScript to "hdiutil create -imagekey zlib-level=9 -srcfolder " & (quoted form of PathToSourceFolder) & " " & (quoted form of PathToCreatedDMGFile) & " -fs HFS+ -volname " & ("'" & NameOfMountedDiscImage & "'")    �  � � � r   � � � � � b   � � � � � b   � � � � � b   � � � � � b   � � � � � b   � � � � � m   � � � � 7 1hdiutil create -imagekey zlib-level=9 -srcfolder     � l  � � ��� � n   � � � � � 1   � ���
�� 
strq � o   � ����� (0 pathtosourcefolder PathToSourceFolder��   � m   � � � �       � l  � � ��� � n   � � � � � 1   � ���
�� 
strq � o   � ����� ,0 pathtocreateddmgfile PathToCreatedDMGFile��   � m   � � � �   -fs HFS+ -volname     � l  � � ��� � n   � � � � � 1   � ���
�� 
strq � o   � ����� 00 nameofmounteddiscimage NameOfMountedDiscImage��   � o      ���� 0 myshellscript myShellScript �  � � � l  � �������  ��   �  � � � l  � ��� ���   � H B	+---------------------------------------------------------------+    �  � � � l  � ��� ���   � 7 1	to create the disk image with a Terminal window,    �  � � � l  � ��� ���   �  	use following lines:    �  � � � l  � �������  ��   �  � � � O   � � � � � I  � ��� ���
�� .coredoscnull        TEXT � o   � ����� 0 myshellscript myShellScript��   � m   � � � ��null     C���� �Terminal.app�� �0�L��� 7��ڰ   Z ��`   )       �(�K� ���� �trmx   alis    d  
Glorfindel                 �㞳H+   �Terminal.app                                                    8��j��        ����  	                	Utilities     �㐣      �j��     � �  .Glorfindel:Applications:Utilities:Terminal.app    T e r m i n a l . a p p   
 G l o r f i n d e l  #Applications/Utilities/Terminal.app   / ��   �  � � � l  � ��� ���   � H B	+---------------------------------------------------------------+    �  � � � l  � �������  ��   �  � � � l  � ��� ���   � H B	+---------------------------------------------------------------+    �  � � � l  � ��� ���   � : 4	to create the disk image without a Terminal window,    �  � � � l  � ��� ���   � " 	use following line instead:    �  � � � l  � �������  ��   �  � � � l  � ��� ���   � $ 	do shell script myShellScript    �  � � � l  � ��� ���   � H B	+---------------------------------------------------------------+    �  ��� � l  � �������  ��  ��  ��  ��  ��  ��   ( I  � ��� � �
�� .sysodlogaskr        TEXT � b   � � � � � b   � � � � � b   � � � � � b   � � � � � b   � � � � � m   � � � � 2 ,The dropped item was not a folder or volume:    � o   � ���
�� 
ret  � o   � ���
�� 
ret  � m   � � � �  '    � l  � � ��� � n   � � � � � 1   � ���
�� 
dnam � o   � �����  0 sourceiteminfo sourceItemInfo��   � m   � � � �  '    � �� � �
�� 
btns � J   � � � �  ��� � m   � � � �  OK   ��   � �� � �
�� 
dflt � m   � �����  � �� � �
�� 
disp � m   � �����  � �� ���
�� 
givu � m   � ����� ��  ��  
�� 
pidx  m    ����   l   
 ��� � n    
 � � � m    	��
�� 
nmbr � n    � � � 2   ��
�� 
cobj � o    ���� 0 droppeditems DroppedItems��  ��     ��� � l     ������  ��  ��       �� � ���   � ��
�� .aevtodocnull  �    alis � �� ���� � ���
�� .aevtodocnull  �    alis�� 0 droppeditems DroppedItems��   � 	����~�}�|�{�z�y�x�� 0 droppeditems DroppedItems
� 
pidx�~ (0 pathtosourcefolder PathToSourceFolder�}  0 sourceiteminfo sourceItemInfo�| &0 kindofdroppeditem kindOfDroppedItem�{ 0 
userchoice 
userChoice�z 00 nameofmounteddiscimage NameOfMountedDiscImage�y ,0 pathtocreateddmgfile PathToCreatedDMGFile�x 0 myshellscript myShellScript � '�w�v�u�t . 2�s B�r C D�q J M O�p�o�n�m�l�k c�j�i } ~ ��h � � ��g � � � ��f�e�d
�w 
cobj
�v 
nmbr
�u .sysonfo4asfe       fss 
�t 
kind
�s 
bool
�r 
ret 
�q 
btns
�p 
dflt
�o 
disp�n 
�m .sysodlogaskr        TEXT
�l 
rslt
�k 
bhit
�j 
dnam
�i 
psxp
�h 
strq
�g .coredoscnull        TEXT
�f 
givu�e �d �� � �k��-�,Ekh ��E/E�O�j E�O��,E�O�� 
 �� �& ���%�%�%�%�%����mv�ma ka  O_ a ,E�O�a   Q�a ,E�O�a ,E�O�a %�%a %E�Oa �a ,%a %�a ,%a %�a ,%E�Oa  �j UOPY hY /a  �%�%a !%�a ,%a "%�a #kv�ka ka $a %a & [OY�& ascr  ��ޭ