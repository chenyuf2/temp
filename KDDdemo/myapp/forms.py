from django import forms
from myapp.fields import ListTextWidget

class ContactForm(forms.Form):
   temp = forms.CharField(label='', widget=forms.TextInput(attrs={'class':'col-lg-7', 'placeholder':'Please type in category names(words or phrases) separated by comma.','autocomplete': 'off'}))

   def __init__(self, *args, **kwargs):
       _country_list = kwargs.pop('data_list', None)
       super(ContactForm, self).__init__(*args, **kwargs)
       self.fields['temp'].widget =ListTextWidget(data_list=_country_list, name='country-list',attrs={'class':'col-lg-7', 'placeholder':'Please type in category names(words or phrases) separated by comma.', 'autocomplete':'off'})
       # self.fields['temp'].widget =forms.TextInput(attrs={'class':'col-lg-7', 'placeholder':'Please type in category names(words or phrases) separated by comma.'})
